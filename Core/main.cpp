#include <iostream>
#include <stdexcept>
#include "src\def.h"
#include "src\graphics\Vulkan\vulkan_base.h"
#include <chrono>

using namespace ng;

class Application {
private:
	graphics::VulkanBase vulkanBase;
	graphics::Window window;
public:

	~Application() {
		cleanup();
	}

	void init() {
		window.init(800, 600, "erik suger");
		initVulkan(&vulkanBase);
	}
	void run() {
		window.run();
	}

	void cleanup() {
		glfwDestroyWindow(window.glfwWindowPtr);
		glfwTerminate();
	}

};


void createInstance(ng::graphics::VulkanBase* vulkanBase) {

}

void initVulkan(ng::graphics::VulkanBase* vulkanBase) {
	createInstance(vulkanBase);
}

void cleanup(ng::graphics::VulkanBase* vulkanBase) {
	
}

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
