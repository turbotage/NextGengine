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

#define NUM_OF_THREADS 4

class ApplicationOld {
private:

	VulkanBase vulkanBase;
	Window window;
	VulkanGraphicsPipeline graphicsPipeline;

	std::vector<std::pair<std::string, NgShaderType>> shaders = {
		std::pair<std::string, NgShaderType>("vert.spv", VERTEX_SHADER_BIT),
		std::pair<std::string, NgShaderType>("frag.spv", FRAGMENT_SHADER_BIT)
	};
	
public:

	~ApplicationOld() {
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
		vulkanBase.createCommandPools();

		printf("successfully went through application initalization\n");
	}

	void run() {
		window.run();
	}

	void cleanup() {
		vulkanBase.freeCommandPools();
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

class Application {
private:

public:

	Window window;

	VulkanBase base;

	VulkanDevice graphicsDevice;
	VulkanDevice computeDevice;

	VulkanSwapchain swapchain;
	
	VulkanFramebuffer framebuffer;
	
public:

	void createPhysicalDevices() {

		uint32 deviceCount = 0;
		vkEnumeratePhysicalDevices(base.instance, &deviceCount, nullptr);

		if (deviceCount == 0) {
			throw std::runtime_error("failed to find physical device with vulkan support");
		}

		std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
		std::vector<VulkanDevice> vulkanDevices(deviceCount);
		std::vector<uint32> scores;
		vkEnumeratePhysicalDevices(base.instance, &deviceCount, physicalDevices.data());

		if (deviceCount == 1) {
			for (uint16 i = 0; i < deviceCount; ++i) {
				vulkanDevices[i].init(physicalDevices[i]);

				VulkanDeviceTypeFlags deviceTypeFlags =
					VULKAN_DEVICE_TYPE_DESCRETE_GRAPHICS_UNIT |
					VULKAN_DEVICE_TYPE_DESCRETE_COMPUTE_UNIT |
					VULKAN_DEVICE_TYPE_HAS_PRESENT_SUPPORT |
					VULKAN_DEVICE_TYPE_HAS_PRESENT_SUPPORT_IN_GRAPHICS_QUEUE;

				scores[i] = vulkanDevices[i].getDeviceScore(deviceTypeFlags, swapchain.surface);
			}

			auto devicePos = std::max_element(scores.begin(), scores.end());
			if (scores[*devicePos] != 0) {
				graphicsDevice = vulkanDevices[*devicePos];
				computeDevice = vulkanDevices[*devicePos];
				return;
			}
			else {
				throw std::runtime_error("found no suitable vulkan-device");
			}
		}
		else if (deviceCount > 1){
			//graphics
			for (uint16 i = 0; i < deviceCount; ++i) {
				vulkanDevices[i].init(physicalDevices[i]);

				VulkanDeviceTypeFlags deviceTypeFlags =
					VULKAN_DEVICE_TYPE_DESCRETE_GRAPHICS_UNIT |
					VULKAN_DEVICE_TYPE_HAS_PRESENT_SUPPORT |
					VULKAN_DEVICE_TYPE_HAS_PRESENT_SUPPORT_IN_GRAPHICS_QUEUE;

				scores[i] = vulkanDevices[i].getDeviceScore(deviceTypeFlags, swapchain.surface);
			}
			auto devicePos = std::max_element(scores.begin(), scores.end());
			if (scores[*devicePos] != 0) {
				graphicsDevice = vulkanDevices[*devicePos];
			}
			else {
				throw std::runtime_error("found no suitable graphics vulkan-device");
			}

			vulkanDevices.erase(vulkanDevices.begin() + *devicePos);

			//compute
			for (uint16 i = 0; i < deviceCount; ++i) {
				vulkanDevices[i].init(physicalDevices[i]);

				VulkanDeviceTypeFlags deviceTypeFlags =
					VULKAN_DEVICE_TYPE_DESCRETE_COMPUTE_UNIT;

				scores[i] = vulkanDevices[i].getDeviceScore(deviceTypeFlags, swapchain.surface);
			}
			auto devicePos2 = std::max_element(scores.begin(), scores.end());
			if (scores[*devicePos2] != 0) {
				computeDevice = vulkanDevices[*devicePos2];
			}
			else {
				for (uint16 i = 0; i < deviceCount; ++i) {
					vulkanDevices[i].init(physicalDevices[i]);

					VulkanDeviceTypeFlags deviceTypeFlags =
						VULKAN_DEVICE_TYPE_DESCRETE_GRAPHICS_UNIT |
						VULKAN_DEVICE_TYPE_DESCRETE_COMPUTE_UNIT |
						VULKAN_DEVICE_TYPE_HAS_PRESENT_SUPPORT |
						VULKAN_DEVICE_TYPE_HAS_PRESENT_SUPPORT_IN_GRAPHICS_QUEUE;

					scores[i] = vulkanDevices[i].getDeviceScore(deviceTypeFlags, swapchain.surface);
				}

				auto devicePos = std::max_element(scores.begin(), scores.end());
				if (scores[*devicePos] != 0) {
					graphicsDevice = vulkanDevices[*devicePos];
					computeDevice = vulkanDevices[*devicePos];
					return;
				}
				else {
					throw std::runtime_error("found no suitable vulkan-device");
				}
			}
		}
	}

	void init() {

		window.init(800, 600, "sick application");

		base.createInstance();
		base.createDebugCallback();

		VulkanSwapchainCreateInfo swapchainCreateInfo = {};
		swapchainCreateInfo.instance = base.instance;
		swapchainCreateInfo.vulkanDevice = nullptr;

		swapchain.init(swapchainCreateInfo);
		swapchain.createSurface(window.glfwWindowPtr);
		
		createPhysicalDevices();
		
		

	}

	void run();

	void cleanup();

};

int main(int argc, char* argv[]){

	using namespace ng::graphics;
	ApplicationOld app;
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


