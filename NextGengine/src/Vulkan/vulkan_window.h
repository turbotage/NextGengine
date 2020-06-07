#pragma once

#include "vulkandef.h"
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>
#include <functional>

namespace ngv {

	class Window {
	public:

		Window() {};

		Window(const vk::Instance& instance, const vk::Device& device,
			const vk::PhysicalDevice& physicalDevice, uint32 graphicsQueueFamilyIndex,
			uint32 width, uint32 height, const char* title);

		void init(const vk::Instance& instance, const vk::Device& device,
			const vk::PhysicalDevice& physicalDevice, uint32 graphicsQueueFamilyIndex, vk::SurfaceKHR surface);

	private:
		GLFWwindow* m_pWindow;

		const char* m_Title;
		uint32 m_Width, m_Height;

		vk::Instance m_Instance;
		vk::SurfaceKHR m_Surface;
		vk::Device m_Device;

		vk::UniqueSwapchainKHR m_Swapchain;
		vk::UniqueRenderPass m_RenderPass;
		vk::UniqueSemaphore m_ImageAcquireSemaphore;
		vk::UniqueSemaphore m_CommandCompleteSemaphore;
		vk::UniqueSemaphore m_DynamicSemaphore;
		
		std::vector<vk::ImageView> m_SwapChainViews;
		std::vector<vk::Image> m_SwapChainImages;
		vk::Format m_SwapChainImageFormat = vk::Format::eB8G8R8A8Snorm;
		vk::ColorSpaceKHR m_SwapChainColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;

		uint32 m_PresentQueueFamily = 0;
		

	};

}