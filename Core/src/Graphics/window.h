#pragma once

#include "../def.h"
#include "vulkan_base.h"
#include <GLFW\glfw3.h>
#include <vector>
//#include "Pipelines\vulkan_graphics_pipeline.h"


/*
Window cordinate system
positive Z is pointing in to the screen
positive X is pointing to the right on the screen
positive Y is pointing downwards on the screen
			    Z
			  -
			-
		  -	
		------------ X
		|
		|
		|
		|
		Y
*/

namespace ng {
	namespace graphics {

		class VulkanGraphicsPipeline;

		class Window {
		private:
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

			void createFramebuffers(VulkanGraphicsPipeline* pipeline);
			void freeFramebuffers();

			void run();

			VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

			VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);

			VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
			
		};
	}
}


