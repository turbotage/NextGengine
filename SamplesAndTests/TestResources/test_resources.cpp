
#include <NextGengineCore.h>


class Model {
public:

	std::shared_ptr<ng::VertexBuffer> vertexBuffer;

};



class BaseApp {
public:

	void init();

	void loadResources();

	void run();

public:

	GLFWwindow* pGLFWWindow;

	std::unique_ptr<ngv::VulkanContext> pContext;

	std::unique_ptr<ngv::VulkanAllocator> pAllocator;

	std::unique_ptr<ngv::VulkanWindow> pWindow;


	vk::RenderPass renderPass;
	vk::PipelineCache cache;
	vk::UniquePipeline pipeline;

	std::unique_ptr<ng::ResourceManager> pResourceManager;


	std::vector<Model> models;

private:


};






int main() {
	BaseApp app;
	app.init();
	app.loadResources();
	app.run();
}












void BaseApp::init()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	const char* title = "Resource Testing";
	bool fullScreen = false;
	int width = 1280;
	int height = 720;
	GLFWmonitor* monitor = nullptr;
	pGLFWWindow = glfwCreateWindow(width, height, title, monitor, nullptr);

	// Context And Window Creation
	pContext = std::unique_ptr<ngv::VulkanContext>(new ngv::VulkanContext(title));
	if (!pContext->ok()) {
		std::cout << "Framework creation failed" << std::endl;
		exit(1);
	}

	std::cout << pContext->vulkanDevice().physicalDeviceProperties().deviceName << std::endl;

	ngv::VulkanMemoryStrategy memStrategy;
	memStrategy.maxMemoryUsage = 2 * 1024 * 1024 * 1024LL;
	memStrategy.recommendedPageSize = 256 * 1024 * 1024;

	pAllocator = std::unique_ptr<ngv::VulkanAllocator>(
		new ngv::VulkanAllocator(pContext->vulkanDevice(), memStrategy));

	pWindow = std::unique_ptr<ngv::VulkanWindow>(new ngv::VulkanWindow(pContext->instance(),
		pContext->vulkanDevice(), *pAllocator, pContext->graphicsQueueFamilyIndex(), pGLFWWindow));
	if (!pWindow->ok()) {
		std::cout << "Window creation failed" << std::endl;
		exit(1);
	}

	//Shaders and pipeline
	std::string shaderPath = ng::getShaderDirectoryPath();

	ngv::VulkanShaderModule vert{ pContext->vulkanDevice().device(), shaderPath + "TestResources/testResources.vert.spv" };
	ngv::VulkanShaderModule frag{ pContext->vulkanDevice().device(), shaderPath + "TestResources/testResources.frag.spv" };

	struct Uniform {
		glm::vec4 color;
		glm::mat4 rotation;
	};

	ngv::VulkanPipelineLayoutMaker plm{};
	plm.addPushConstantRange(vk::ShaderStageFlagBits::eAll, 0, sizeof(Uniform));
	auto pipelineLayout = plm.createUnique(pContext->vulkanDevice().device());

	struct Vertex { glm::vec2 pos; glm::vec3 color; };

	ngv::VulkanPipelineMaker pm{ (uint32)width, (uint32)height };
	pm.addShader(vk::ShaderStageFlagBits::eVertex, vert);
	pm.addShader(vk::ShaderStageFlagBits::eFragment, frag);
	pm.addVertexBinding(0, (uint32)sizeof(Vertex));
	pm.addVertexAttribute(0, 0, vk::Format::eR32G32Sfloat, (uint32)offsetof(Vertex, pos));
	pm.addVertexAttribute(1, 0, vk::Format::eR32G32B32Sfloat, (uint32)offsetof(Vertex, color));

	renderPass = pWindow->renderPass();
	cache = pContext->pipelineCache();
	pipeline = pm.createUnique(pContext->vulkanDevice().device(), cache, *pipelineLayout, renderPass);

}

void BaseApp::loadResources()
{
	ng::ResourceStrategy resourceStrategy;
	
	pResourceManager = std::unique_ptr<ng::ResourceManager>(
		new ng::ResourceManager(*pAllocator, pContext->vulkanDevice(), resourceStrategy));

	auto assetPath = ng::getAssetsDirectoryPath() + "\\test_assets\\";

	Model temp;
	temp.vertexBuffer = pResourceManager->getVertexBuffer(assetPath + "test_vert2D_3C.v2d3c");


}

void BaseApp::run()
{
	while (!glfwWindowShouldClose(pGLFWWindow)) {
		glfwPollEvents();



	}
}
