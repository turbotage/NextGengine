



#include <NextGengineCore.h>

class BaseApp {
public:

	void Init();

	void LoadResources();

	void Run();

public:

	GLFWwindow* pGLFWWindow;

	std::unique_ptr<ngv::VulkanContext> pContext;

	std::unique_ptr<ngv::VulkanAllocator> pAllocator;

	std::unique_ptr<ngv::VulkanWindow> pWindow;

	vk::RenderPass renderPass;
	vk::PipelineCache cache;
	vk::UniquePipeline pipeline;

	std::unique_ptr<ng::ResourceManager> pResourceManager;


	std::unique_ptr<ng::Scene3D> pScene;



};

void BaseApp::Init()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	const char* title = "Resource Testing 2";
	bool fullScreen = false;
	int width = 1200;
	int height = 720;
	GLFWmonitor* monitor = nullptr;
	pGLFWWindow = glfwCreateWindow(width, height, title, monitor, nullptr);

	pContext = std::unique_ptr<ngv::VulkanContext>(new ngv::VulkanContext(title));
	if (!pContext->ok()) {
		std::cout << "Framework creation failed" << std::endl;
		exit(1);
	}

	std::cout << pContext->vulkanDevice().physicalDeviceProperties().deviceName << std::endl;

	ngv::VulkanMemoryStrategy memStrategy;
	memStrategy.maxMemoryUsage = 2 * 1024 * 1024 * 1024LL;
	memStrategy.recommendedPageSize = 256 * 1024 * 1024LL;

	pAllocator = std::unique_ptr<ngv::VulkanAllocator>(new ngv::VulkanAllocator(pContext->vulkanDevice(), memStrategy));

	pWindow = std::unique_ptr<ngv::VulkanWindow>(new ngv::VulkanWindow(pContext->instance(), 
		pContext->vulkanDevice(), *pAllocator, pContext->graphicsQueueFamilyIndex(), pGLFWWindow));
	if (!pWindow->ok()) {
		std::cout << "Window creation failed" << std::endl;
		exit(1);
	}

	std::string shaderPath = ng::getShaderDirectoryPath();

	ngv::VulkanShaderModule vert{ pContext->vulkanDevice().device(), shaderPath + "TestResources/testResources.vert.spv" };
	ngv::VulkanShaderModule frag{ pContext->vulkanDevice().device(), shaderPath + "TestResources/testResources.frag.spv" };

	struct Uniform {
		glm::vec4 color;
		glm::mat4 rotation;
	};

	ngv::VulkanPipelineLayoutMaker plm{};



}

void BaseApp::LoadResources()
{
}

void BaseApp::Run()
{
}
