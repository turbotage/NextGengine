#include <iostream>
#include <stdexcept>
#include "src\def.h"
#include "src\graphics\Vulkan\vulkan_base.h"
#include <chrono>


void createInstance(ng::graphics::VulkanBase* vulkanBase) {
	

}

void initVulkan(ng::graphics::VulkanBase* vulkanBase) {
	createInstance(vulkanBase);
}

void cleanup(ng::graphics::VulkanBase* vulkanBase) {



	glfwDestroyWindow(vulkanBase->window.glfwWindowPtr);

	glfwTerminate();
}

int main(int argc, char* argv[]){
	using namespace ng::graphics;
	VulkanBase vulkanBase;
	try {
		vulkanBase.window.init(800, 600, "erik suger");
		initVulkan(&vulkanBase);
		vulkanBase.window.run();
		cleanup(&vulkanBase);
	}
	catch (std::runtime_error e) {
		std::cout << e.what() << std::endl;
	}
	getchar();
	return 0;
}
