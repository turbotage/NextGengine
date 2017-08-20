#include <iostream>
#include <stdexcept>
#include "src\def.h"
#include "src\graphics\Vulkan\vulkan_base.h"
#include "src\debug.h"
#include <chrono>

using namespace ng;


void initVulkan(ng::graphics::VulkanBase* vulkanBase, graphics::Window* window) {
	vulkanBase->createInstance();
	vulkanBase->createDebugCallback();
	window->createSurface(&vulkanBase->instance, &vulkanBase->surface);
	vulkanBase->createDevices();
	
}

class Application {
private:
	graphics::VulkanBase vulkanBase;
	graphics::Window window;
public:

	~Application() {
		cleanup();
	}

	void init() {
		window.init(800, 600, "window");
		initVulkan(&vulkanBase, &window);
	}
	void run() {
		window.run();
	}

	void cleanup() {
		vkDestroyInstance(vulkanBase.instance, nullptr);
		glfwDestroyWindow(window.glfwWindowPtr);
		glfwTerminate();
	}

};

int main(int argc, char* argv[]){
	
	using namespace ng::graphics;
	Application app;
	try {
		app.init();
		app.run();
		app.cleanup();
	}
	catch (std::runtime_error e) {
		std::cout << e.what() << std::endl;
	}
	getchar();
	return 0;
}


