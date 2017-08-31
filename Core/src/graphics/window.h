#pragma once

#include "../def.h"
#include "vulkan\vulkan_base.h"
#include <GLFW\glfw3.h>
#include <vector>

namespace ng {
	namespace graphics {
		class Window {
			VkInstance* m_Instance;
			VkSurfaceKHR* m_Surface;
			VkDevice* m_Device;
			uint16 m_Width;
			uint16 m_Height;

		public:
			GLFWwindow* glfwWindowPtr;

			VkSwapchainKHR swapChain;
			std::vector<VkImage> swapChainImages;
			VkFormat swapChainImageFormat;
			VkExtent2D swapChainExtent;
			std::vector<VkImageView> swapChainImageViews;
			std::vector<VkFramebuffer> swapChainFramebuffers;

		public:

			void init(uint width, uint height, const char* description);

			void createSurface(VkInstance* instance, VkSurfaceKHR* surface);
			void freeSurface();

			void createSwapChain(VkDevice* device, SwapChainSupportDetails swapChainSupport, QueueFamilyIndices indices);
			void freeSwapChain();

			void createSwapChainImageViews();
			void freeSwapChainImageViews();

			void run();

			VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

			VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);

			VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
			
		};
	}
}


