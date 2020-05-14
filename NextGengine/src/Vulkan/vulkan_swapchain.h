#pragma once

#include "vulkandef.h"

namespace ngv {

	struct VulkanSwapChainImage {
		vk::Image image;
		vk::ImageView view;
		vk::Fence fence;
	};

	class VulkanSwapChain {
	public:
		vk::SurfaceKHR surface;
		vk::SwapchainKHR swapChain;
		vk::PresentInfoKHR presentInfo;
		vk::PhysicalDevice physicalDevice;
		vk::Device device;
		vk::Queue queue;
		std::vector<VulkanSwapChainImage> images;
		vk::Format colorFormat;
		vk::ColorSpaceKHR colorSpace;
		uint32 imageCount{ 0 };
		uint32 currentImage{ 0 };
		uint32 graphicsQueueIndex{ VK_QUEUE_FAMILY_IGNORED };


		VulkanSwapChain() {
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = &swapChain;
			presentInfo.pImageIndices = &currentImage;
		}


		void setup(const vk::PhysicalDevice& newPhysicalDevice, 
			const vk::Device& newDevice, 
			const vk::Queue& newQueue, 
			uint32 newGraphicsQueueIndex);

		void setSurface(const vk::SurfaceKHR& newSurface);

		void create(vk::Extent2D& size, bool vsync = false);

		std::vector<vk::Framebuffer> createFramebuffers(vk::FramebufferCreateInfo framebufferCreateInfo);

		vk::ResultValue<uint32_t> acquireNextImage(const vk::Semaphore& presentCompleteSemaphore, const vk::Fence& fence = vk::Fence());

		void clearSubmitFence(uint32_t index);

		vk::Fence getSubmitFence(bool destroy = false);

		// Present the current image to the queue
		vk::Result queuePresent(vk::Semaphore waitSemaphore);

		// Free all Vulkan resources used by the swap chain
		void destroy();

	private:

		uint32_t findQueue(const vk::QueueFlags& flags) const;

	};

}