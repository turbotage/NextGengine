#include "../NextGengine/NextGengine.h"

#include <iostream>
#include <filesystem>

int main() {
	
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	const char* title = "Hello Triangle";
	bool fullScreen = false;
	int width = 800;
	int height = 600;
	GLFWmonitor* monitor = nullptr;
	auto glfwWindow = glfwCreateWindow(width, height, title, monitor, nullptr);

	{
		ngv::VulkanContext context{ title };
		if (!context.ok()) {
			std::cout << "Framework creation failed" << std::endl;
			exit(1);
		}

		ngv::VulkanDevice& device = context.vulkanDevice();

		ngv::VulkanMemoryStrategy memStrategy;
		memStrategy.maxMemoryUsage = 2 * 1024 * 1024 * 1024LL;
		memStrategy.recommendedPageSize = 256 * 1024 * 1024;

		ngv::VulkanAllocator allocator{ device, memStrategy };


		ngv::VulkanWindow window{ context.instance(), device, allocator, context.graphicsQueueFamilyIndex(), glfwWindow };
		if (!window.ok()) {
			std::cout << "Window creation failed" << std::endl;
			exit(1);
		}

		//ngv::VulkanShaderModule vertShader{ device.device(), "" };
		
		namespace fs = std::filesystem;

		std::cout << ng::listShaders("NextGengine") << std::endl;

		std::string a;
		std::cin >> a;

	}

}