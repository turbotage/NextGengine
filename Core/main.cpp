#include <iostream>
#include <stdexcept>
#include "src\def.h"
#include "src\Graphics\vulkan_base.h"
#include "src\Graphics\window.h"
#include "src\debug.h"
#include <chrono>
#include "src\Math\mat4.h"
#include "src\Graphics\Pipelines\vulkan_graphics_pipeline.h"
#include "src\Memory\VirtualMemoryAllocators\allocator.h"

using namespace ng::graphics;

class Application {
private:
	VulkanBase vulkanBase;
	Window window;
	VulkanGraphicsPipeline graphicsPipeline;

	std::vector<std::pair<std::string, NgShaderType>> shaders = {
		std::pair<std::string, NgShaderType>("vert.spv", VERTEX_SHADER_BIT),
		std::pair<std::string, NgShaderType>("frag.spv", FRAGMENT_SHADER_BIT)
	};
	
public:

	~Application() {
		cleanup();
	}

	void startCommandBuffers(std::vector<VkCommandBuffer>* buffers) {
		for (uint32 i = 0; i < (*buffers).size(); ++i) {
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
			beginInfo.pInheritanceInfo = nullptr;
			vkBeginCommandBuffer((*buffers)[i], &beginInfo);
		}
	}

	void init() {
		window.init(800, 600, "window");
		vulkanBase.createInstance();
		vulkanBase.createDebugCallback();
		window.createSurface(&vulkanBase.instance, &vulkanBase.surface);
		vulkanBase.createPhysicalDevices();
		vulkanBase.createLogicalDevices();
		window.createSwapChain(
			&vulkanBase.graphicsUnit.device, 
			vulkanBase.querySwapChainSupport(
				vulkanBase.graphicsUnit.pDevice.device), 
				vulkanBase.findQueueFamilies(
					vulkanBase.graphicsUnit.pDevice.device
				)
			);
		window.createSwapChainImageViews();
		graphicsPipeline.createRenderPass(&vulkanBase.graphicsUnit.device, &window);
		graphicsPipeline.createGraphicsPipeline(&shaders);
		window.createFramebuffers(&graphicsPipeline);
		
		printf("successfully went through application initalization\n");
	}

	void run() {
		window.run();
	}

	void cleanup() {
		window.freeFramebuffers();
		graphicsPipeline.freeGraphicsPipeline();
		graphicsPipeline.freeRenderPass();
		window.freeSwapChainImageViews();
		window.freeSwapChain();
		vulkanBase.freeLogicalDevices();
		vulkanBase.freeDebugCallback();
		window.freeSurface();
		vulkanBase.freeInstance();
		glfwDestroyWindow(window.glfwWindowPtr);
		glfwTerminate();
	}

};

int main(int argc, char* argv[]){

	char array[50];
	for (int i = 0; i < 50; ++i) {
		array[i] = 0;
	}
	ng::memory::Allocator allocator;
	allocator.init(50);
	std::vector<ng::memory::Allocation> allocs;
	char c = 'a';
	do {
		if (c == 'a') {
			printf("allocation size\n");
			std::cin >> c;
			ng::memory::Allocation a = allocator.allocate(c - '0');
			for (int i = 0; i < c-'0'; i++) {
				array[a.offset + i] = a.allocationIndex + '0';
			}
			allocs.push_back(a);
		}
		else if (c == 'f') {
			printf("allocation to free\n");
			std::cin >> c;
			ng::memory::Allocation a = allocs[c-'0'];
			for (int i = 0; i < a.size; ++i) {
				array[a.offset + i] = 0;
			}
			allocator.freeAllocation(&a);
		}
		else if (c == 'g') {
			allocator.getAllocatedMemory();
		}
		for (int i = 0; i < 50; ++i) {
			printf("%c", array[i]);
		}
		std::cin >> c;
	} while (c != 'q');

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


