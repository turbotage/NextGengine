#include <iostream>
#include <stdexcept>
#include "src\def.h"
#include "src\Graphics\Vulkan\vulkan_base.h"
#include "src\debug.h"
#include <chrono>
#include "src\Math\dyn_mat.h"

using namespace ng;


class Application {
private:
	graphics::Window window;
	graphics::VulkanBase vulkanBase;
public:

	~Application() {
		cleanup();
	}

	void init() {
		window.init(800, 600, "window");
		vulkanBase.createInstance();
		vulkanBase.createDebugCallback();
		window.createSurface(&vulkanBase.instance, &vulkanBase.surface);
		vulkanBase.createPhysicalDevices();
		vulkanBase.createLogicalDevices();
	}

	void run() {
		window.run();
	}

	void cleanup() {
		vulkanBase.freeLogicalDevices();
		vulkanBase.freeDebugCallback();
		vulkanBase.freeInstance();
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
	}
	catch (std::runtime_error e) {
		std::cout << e.what() << std::endl;
	}
	getchar();
	return 0;
}


