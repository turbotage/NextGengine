
#include "../NextGengine/NextGengine.h"

#include <iostream>
#include <filesystem>
#include <memory>

void run() {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	const char* title = "Push Constants";
	bool fullScreen = false;
	int width = 1280;
	int height = 720;
	GLFWmonitor* monitor = nullptr;
	auto glfwWindow = glfwCreateWindow(width, height, title, monitor, nullptr);

	// Context And Window Creation
	ngv::VulkanContext context{ title };
	if (!context.ok()) {
		std::cout << "Framework creation failed" << std::endl;
		exit(1);
	}

	ngv::VulkanDevice& device = context.vulkanDevice();

	std::cout << device.physicalDeviceProperties().deviceName << std::endl;

	ngv::VulkanMemoryStrategy memStrategy;
	memStrategy.maxMemoryUsage = 2 * 1024 * 1024 * 1024LL;
	memStrategy.recommendedPageSize = 256 * 1024 * 1024;

	ngv::VulkanAllocator allocator{ device, memStrategy };


	ngv::VulkanWindow window{ context.instance(), device, allocator, context.graphicsQueueFamilyIndex(), glfwWindow };
	if (!window.ok()) {
		std::cout << "Window creation failed" << std::endl;
		exit(1);
	}


	//Shaders and Pipeline

	std::string shaderPath = ng::getShaderDirectoryPath("NextGengine");

	ngv::VulkanShaderModule vert{ device.device(), shaderPath + "/PushConstants/pushConstants.vert.spv" };
	ngv::VulkanShaderModule frag{ device.device(), shaderPath + "/PushConstants/pushConstants.frag.spv" };

	struct Uniform {
		glm::vec4 color;
		glm::mat4 rotation;
	};

	ngv::VulkanPipelineLayoutMaker plm{};
	plm.addPushConstantRange(vk::ShaderStageFlagBits::eAll, 0, sizeof(Uniform));
	auto pipelineLayout = plm.createUnique(device.device());

	struct Vertex { glm::vec2 pos; glm::vec3 color; };

	const std::vector<Vertex> vertices = {
		{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
		{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
		{{0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
		{{-0.5f, -0.5f}, { 1.0f, 0.0f, 0.0f}},
		{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
		{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
	};


	vk::DeviceSize bufferSize = vertices.size() * sizeof(Vertex);
	std::shared_ptr<ngv::VertexBuffer> pBuffer = ngv::VertexBuffer::make(device, bufferSize, true);
	allocator.giveBufferAllocation(pBuffer);
	pBuffer->updateLocal(vertices);

	ngv::VulkanPipelineMaker pm{ (uint32)width, (uint32)height };
	pm.addShader(vk::ShaderStageFlagBits::eVertex, vert);
	pm.addShader(vk::ShaderStageFlagBits::eFragment, frag);
	pm.addVertexBinding(0, (uint32)sizeof(Vertex));
	pm.addVertexAttribute(0, 0, vk::Format::eR32G32Sfloat, (uint32)offsetof(Vertex, pos));
	pm.addVertexAttribute(1, 0, vk::Format::eR32G32B32Sfloat, (uint32)offsetof(Vertex, color));

	auto renderPass = window.renderPass();
	auto& cache = context.pipelineCache();
	auto pipeline = pm.createUnique(device.device(), cache, *pipelineLayout, renderPass);

	Uniform u;
	u.color = glm::vec4{ 1,1,1,1 };
	u.rotation = glm::mat4{ 1 };

	int frame = 0;

	while (!glfwWindowShouldClose(glfwWindow)) {
		glfwPollEvents();

		u.rotation = glm::rotate(u.rotation, glm::radians(1.0f), glm::vec3(0, 0, 1));
		u.color.r = std::sin(frame * 0.01f);
		u.color.g = std::cos(frame * 0.01f);
		frame++;

		window.draw(context.graphicsQueue(), [&](vk::CommandBuffer cb, int imageIndex, vk::RenderPassBeginInfo& rpbi)
			{
				vk::CommandBufferBeginInfo bi{};
				cb.begin(bi);
				cb.pushConstants(*pipelineLayout, vk::ShaderStageFlagBits::eAll, 0, sizeof(u), (const void*)&u);
				cb.beginRenderPass(rpbi, vk::SubpassContents::eInline);
				cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);
				cb.bindVertexBuffers(0, pBuffer->buffer(), (vk::DeviceSize)0);
				cb.draw(6, 1, 0, 0);
				cb.endRenderPass();
				cb.end();
			}
		);

		// crude way to produce less frames
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}

	device.device().waitIdle();


	glfwDestroyWindow(glfwWindow);

	glfwTerminate();

}

int main() {

	run();

	std::string a;
	std::cin >> a;

	return 0;
}
