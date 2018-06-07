#include <iostream>
#include <stdexcept>
#include "src\def.h"
#include "src\Vulkan\vulkan_base.h"
#include "src\Vulkan\window.h"
#include "src\debug.h"
#include <chrono>
#include "src\Math\mat4f.h"
#include "src\Vulkan\Pipelines\vulkan_graphics_pipeline.h"

using namespace ng::vulkan;

#define NUOF_THREADS 4

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

				std::vector<const char*> deviceExtensions(graphicsDeviceExtensions.size() + computeDeviceExtensions.size());

				for (auto extension : graphicsDeviceExtensions) {
					deviceExtensions.push_back(extension);
				}
				for (auto extension : computeDeviceExtensions) {
					deviceExtensions.push_back(extension);
				}

				scores[i] = vulkanDevices[i].getDeviceScore(deviceTypeFlags, deviceExtensions, swapchain.surface);
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

				scores[i] = vulkanDevices[i].getDeviceScore(deviceTypeFlags, graphicsDeviceExtensions, swapchain.surface);
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

				scores[i] = vulkanDevices[i].getDeviceScore(deviceTypeFlags, computeDeviceExtensions, swapchain.surface);
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

					std::vector<const char*> deviceExtensions(graphicsDeviceExtensions.size() + computeDeviceExtensions.size());

					for (auto extension : graphicsDeviceExtensions) {
						deviceExtensions.push_back(extension);
					}
					for (auto extension : computeDeviceExtensions) {
						deviceExtensions.push_back(extension);
					}

					scores[i] = vulkanDevices[i].getDeviceScore(deviceTypeFlags, deviceExtensions, swapchain.surface);
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

		base.createInstance(window.getWindowRequiredExtensions());
		base.createDebugCallback();

		VulkanSwapchainCreateInfo swapchainCreateInfo = {};
		swapchainCreateInfo.instance = base.instance;
		swapchainCreateInfo.vulkanDevice = nullptr;

		swapchain.init(swapchainCreateInfo);
		swapchain.createSurface(window.glfwWindowPtr);
		
		createPhysicalDevices();
		
		VkPhysicalDeviceFeatures graphicsDeviceFeatures = {};
		VkPhysicalDeviceFeatures computeDeviceFeatures = {};

		graphicsDevice.createLogicalDevice(graphicsDeviceFeatures, graphicsDeviceExtensions, VK_QUEUE_GRAPHICS_BIT, swapchain.surface);
		computeDevice.createLogicalDevice(computeDeviceFeatures, computeDeviceExtensions, VK_QUEUE_COMPUTE_BIT);

		//set swapchain device before swapchain creation
		swapchainCreateInfo.vulkanDevice = &graphicsDevice;
		swapchain.init(swapchainCreateInfo);
		swapchain.createSwapchain(&window.width, &window.width);



	}

	void run();

	void cleanup();

};

int main(int argc, char* argv[]){

	using namespace ng::vulkan;

	getchar();
	return 0;
}


