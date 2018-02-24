#pragma once

#include "../def.h"
#include "vulkan_device.h"

namespace ng {
	namespace graphics {

		struct SwapchainSupportDetails {
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

		struct VulkanSwapchainCreateInfo {
			VkInstance instance;
			VulkanDevice* vulkanDevice;
		};

		struct SwapchainBuffer {
			VkImage image;
			VkImageView view;
		};

		class VulkanSwapchain
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
			std::vector<SwapchainBuffer> buffers;

			uint32 queueNodeIndex = UINT32_MAX;

		public:

			VulkanSwapchain(VulkanSwapchainCreateInfo createInfo);
			~VulkanSwapchain();

			void createSurface(GLFWwindow *glfwWindowPtr);

			void createSwapchain(uint32* width, uint32* height, bool vsync = false);

			VkResult acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32 *imageIndex);

			VkResult queuePresent(VkQueue queue, uint32 imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);

			void freeSwapchain();

			SwapchainSupportDetails querySwapchainSupport();

		};
	}
}


