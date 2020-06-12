#pragma once

#include "vulkandef.h"
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>
#include <functional>

namespace ngv {

	class VulkanDevice;
	class DepthStencilImage;
	class VulkanAllocator;

	class Window {
	public:

		Window(const vk::Instance& instance, VulkanDevice& device, VulkanAllocator& allocator, uint32 graphicsQueueFamilyIndex, GLFWwindow* pWindow);

	private:
		
		void init();

	private:

		const char* m_Title;
		uint32 m_Width, m_Height;

		vk::Instance m_Instance;
		vk::SurfaceKHR m_Surface;

		VulkanDevice& m_Device;
		VulkanAllocator& m_Allocator;

		vk::UniqueSwapchainKHR m_Swapchain;
		vk::UniqueRenderPass m_RenderPass;
		vk::UniqueSemaphore m_ImageAcquireSemaphore;
		vk::UniqueSemaphore m_CommandCompleteSemaphore;
		vk::UniqueSemaphore m_DynamicSemaphore;
		
		std::vector<vk::ImageView> m_SwapChainViews;
		std::vector<vk::Image> m_SwapChainImages;
		std::vector<vk::UniqueFramebuffer> m_Framebuffers;
		vk::Format m_SwapChainImageFormat = vk::Format::eB8G8R8A8Snorm;
		vk::ColorSpaceKHR m_SwapChainColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;

		uint32 m_GraphicsQueueFamilyIndex = 0;
		uint32 m_PresentQueueFamilyIndex = 0;
		
		std::shared_ptr<DepthStencilImage> m_DepthStencil;


	};

}