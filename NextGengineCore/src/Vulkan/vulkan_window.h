#pragma once

#include "pch.h"


class GLFWwindow;

namespace ngv {

	class VulkanDevice;
	class VulkanDepthStencilImage;
	class VulkanAllocator;

	class VulkanWindow {
	public:

		VulkanWindow(const vk::Instance& instance, VulkanDevice& device, VulkanAllocator& allocator, uint32 graphicsQueueFamilyIndex, GLFWwindow* pWindow);

		void dumpCaps(std::ostream& os, vk::PhysicalDevice pd) const;

		void draw(const vk::Queue& graphicsQueue, const std::function<void(vk::CommandBuffer cb, uint32 imageIndex, vk::RenderPassBeginInfo& rpbi)>& renderFunc);

		uint32 presentQueueFamilyIndex() const;

		const vk::Queue presentQueue() const;

		bool ok() const;

		vk::RenderPass renderPass() const;

		const std::vector<vk::UniqueFramebuffer>& framebuffers() const;

		~VulkanWindow();

		VulkanWindow& operator=(VulkanWindow&& rhs) = default;

		uint32 width() const;
		uint32 height() const;

		// Return the height of the display
		vk::Format swapchainImageFormat() const;
		// Return the format of the backbuffer
		vk::ColorSpaceKHR swapchainColorSpace() const;
		// Return the swapchain object
		const vk::SwapchainKHR swapchain() const;
		// Return the views of the swapchain images
		const std::vector<vk::ImageView>& imageViews() const;
		// Return the swap chain images
		const std::vector<vk::Image>& images() const;
		// Return the static command buffers
		const std::vector<vk::UniqueCommandBuffer>& commandBuffers() const;
		// Return the fences used to control the buffers
		const std::vector<vk::Fence>& commandBufferFences() const;
		// Return the semaphore signalled when an image is acquired
		vk::Semaphore imageAcquireSemaphore() const;
		// Return the semaphore signalled when the command buffers are finished
		vk::Semaphore commandCompleteSemaphore() const;
		// Return a default command pool to use to create new commandBuffers
		vk::CommandPool commandPool() const;

		uint32 numSwapchainImages() const;

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

		std::vector<vk::ImageView> m_SwapChainViews;
		std::vector<vk::Image> m_SwapChainImages;
		std::vector<vk::UniqueFramebuffer> m_Framebuffers;
		vk::Format m_SwapChainImageFormat = vk::Format::eB8G8R8A8Snorm;
		vk::ColorSpaceKHR m_SwapChainColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;

		vk::UniqueCommandPool m_CommandPool;
		std::vector<vk::UniqueCommandBuffer> m_CommandBuffers;
		std::vector<vk::Fence> m_CommandBufferFences;


		uint32 m_GraphicsQueueFamilyIndex = 0;
		uint32 m_PresentQueueFamilyIndex = 0;
		
		std::shared_ptr<VulkanDepthStencilImage> m_DepthStencil;

		bool m_Ok = false;
	};

}