#include <iostream>
#include <stdexcept>
#include "src\def.h"
#include "src\Graphics\Vulkan\vulkan_base.h"
#include "src\Graphics\window.h"
#include "src\debug.h"
#include <chrono>
#include "src\Math\dyn_mat.h"

using namespace ng::graphics;


class Application : VulkanBase {
private:
	Window window;
public:

	~Application() {
		cleanup();
	}

	void init() {
		window.init(800, 600, "window");
		createInstance();
		createDebugCallback();
		window.createSurface(&instance, &surface);
		createPhysicalDevices();
		createLogicalDevices();
		window.createSwapChain(&graphicsUnit.device, querySwapChainSupport(graphicsUnit.pDevice.device), findQueueFamilies(graphicsUnit.pDevice.device));
		window.createSwapChainImageViews();
		printf("successfully went through application initalization\n");
	}

	void run() {
		window.run();
	}

	void cleanup() {
		window.freeSwapChainImageViews();
		window.freeSwapChain();
		freeLogicalDevices();
		freeDebugCallback();
		window.freeSurface();
		freeInstance();
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


