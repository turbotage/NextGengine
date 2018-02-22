#pragma once

#include "../def.h"
#include "vulkan_device.h"

namespace ng {
	namespace graphics {

		struct VulkanSwapChainCreateInfo {
			VkInstance instance;
			VulkanDevice* vulkanDevice;
		};

		struct SwapChainBuffer {
			VkImage image;
			VkImageView view;
		};

		class VulkanSwapChain
		{
		private:
			VkInstance instance;
			VulkanDevice* vulkanDevice;

			// Function pointers
			PFN_vkGetPhysicalDeviceSurfaceSupportKHR fpGetPhysicalDeviceSurfaceSupportKHR;
			PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
			PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fpGetPhysicalDeviceSurfaceFormatsKHR;
			PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fpGetPhysicalDeviceSurfacePresentModesKHR;
			PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR;
			PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR;
			PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;
			PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;
			PFN_vkQueuePresentKHR fpQueuePresentKHR;
		public:
			VkFormat colorFormat;
			VkColorSpaceKHR colorSpace;
			
			VkSurfaceKHR surface;
			VkSwapchainKHR swapChain;

			uint32 imageCount;
			std::vector<VkImage> images;
			std::vector<SwapChainBuffer> buffers;

			uint32 queueNodeIndex = UINT32_MAX;

		public:

			VulkanSwapChain();
			~VulkanSwapChain();

			void createSurface(GLFWwindow *glfwWindowPtr);

			void createSwapChain(uint32* width, uint32* height, bool vsync = false);

			VkResult acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32 *imageIndex);

			VkResult queuePresent(VkQueue queue, uint32 imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);

			void cleanup();

		};
	}
}


