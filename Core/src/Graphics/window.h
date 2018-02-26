#pragma once

#include "../def.h"
#include "vulkan_base.h"
#include "vulkan_swapchain.h"
#include "vulkan_framebuffer.h"
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

		public:
			GLFWwindow* glfwWindowPtr;

			uint32 width;
			uint32 height;

		public:

			void init(uint32 width, uint32 height, const char* description);

			//void createFramebuffers(VulkanGraphicsPipeline* pipeline);
			//void freeFramebuffers();

			//void run();

			//VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

			//VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);

			//VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
			
		};
	}
}


