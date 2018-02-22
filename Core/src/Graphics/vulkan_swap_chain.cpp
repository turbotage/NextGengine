#include "vulkan_swap_chain.h"

void ng::graphics::VulkanSwapChain::createSurface(GLFWwindow *glfwWindowPtr)
{
	VULKAN_CHECK_RESULT(glfwCreateWindowSurface(instance, glfwWindowPtr, nullptr, &surface));
}

void ng::graphics::VulkanSwapChain::createSwapChain(uint32* width, uint32* height, bool vsync)
{
	VkSwapchainKHR oldSwapChain = swapChain;

	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	VULKAN_CHECK_RESULT(fpGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkanDevice->physicalDevice, surface, &surfaceCapabilities));

	uint32 presentModeCount;
	VULKAN_CHECK_RESULT(fpGetPhysicalDeviceSurfacePresentModesKHR(vulkanDevice->physicalDevice, surface, &presentModeCount, NULL));
	assert(presentModeCount > 0);

	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	VULKAN_CHECK_RESULT(fpGetPhysicalDeviceSurfacePresentModesKHR(vulkanDevice->physicalDevice, surface, &presentModeCount, presentModes.data()));

	VkExtent2D swapchainExtent = {};
	if (surfaceCapabilities.currentExtent.width == (uint32)-1) {
		swapchainExtent.width = *width;
		swapchainExtent.height = *height;
	}
	else {
		swapchainExtent = surfaceCapabilities.currentExtent;
		*width = surfaceCapabilities.currentExtent.width;
		*height = surfaceCapabilities.currentExtent.height;
	}

	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

	if (!vsync) {
		for (size_t i = 0; i < presentModeCount; ++i) {
			if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
				swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR;
				break;
			}
			if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)) {
				swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			}
		}
	}

	uint32 desiredNumberOfSwapchainImages = surfaceCapabilities.minImageCount + 1;
	if ((surfaceCapabilities.maxImageCount > 0) && (desiredNumberOfSwapchainImages > surfaceCapabilities.maxImageCount)) {
		desiredNumberOfSwapchainImages = surfaceCapabilities.maxImageCount;
	}

	VkSurfaceTransformFlagsKHR preTransform;
	if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else {
		preTransform = surfaceCapabilities.currentTransform;
	}

	VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
	};

	for (auto& compositeAlphaFlag : compositeAlphaFlags) {
		if (surfaceCapabilities.supportedCompositeAlpha & compositeAlphaFlag) {
			compositeAlpha = compositeAlphaFlag;
			break;
		}
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.pNext = NULL;
	createInfo.surface = surface;
	createInfo.minImageCount = desiredNumberOfSwapchainImages;
	createInfo.imageFormat = colorFormat;
	createInfo.imageColorSpace = colorSpace;
	createInfo.imageExtent = {swapchainExtent.width, swapchainExtent.height};
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
	createInfo.imageArrayLayers = 1;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.queueFamilyIndexCount = 0;
	createInfo.pQueueFamilyIndices = NULL;
	createInfo.presentMode = swapchainPresentMode;
	createInfo.oldSwapchain = oldSwapChain;
	createInfo.clipped = VK_TRUE;
	createInfo.compositeAlpha = compositeAlpha;

	VkFormatProperties formatProps;
	vkGetPhysicalDeviceFormatProperties(vulkanDevice->physicalDevice, colorFormat, &formatProps);
	if ((formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT_KHR) || 
		(formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT)) 
	{
		createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}

	VULKAN_CHECK_RESULT(fpCreateSwapchainKHR(vulkanDevice->logicalDevice, &createInfo, nullptr, &swapChain));

	if (oldSwapChain != VK_NULL_HANDLE) {
		for (uint32 i = 0; i < imageCount; ++i) {
			vkDestroyImageView(vulkanDevice->logicalDevice, buffers[i].view, nullptr);
		}
		fpDestroySwapchainKHR(vulkanDevice->logicalDevice, oldSwapChain, nullptr);
	}

	VULKAN_CHECK_RESULT(fpGetSwapchainImagesKHR(vulkanDevice->logicalDevice, swapChain, &imageCount, images.data()));

	buffers.resize(imageCount);
	VULKAN_CHECK_RESULT(fpGetSwapchainImagesKHR(vulkanDevice->logicalDevice, swapChain, &imageCount, images.data()));

	buffers.resize(imageCount);
	for (uint32 i = 0; i < imageCount; ++i) {
		VkImageViewCreateInfo colorAttachmentView = {};
		colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		colorAttachmentView.pNext = NULL;
		colorAttachmentView.format = colorFormat;
		colorAttachmentView.components = {
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A
		};
		colorAttachmentView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		colorAttachmentView.subresourceRange.baseMipLevel = 0;
		colorAttachmentView.subresourceRange.levelCount = 1;
		colorAttachmentView.subresourceRange.baseArrayLayer = 0;
		colorAttachmentView.subresourceRange.layerCount = 1;
		colorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
		colorAttachmentView.flags = 0;

		buffers[i].image = images[i];

		colorAttachmentView.image = buffers[i].image;

		VULKAN_CHECK_RESULT(vkCreateImageView(vulkanDevice->logicalDevice, &colorAttachmentView, nullptr, &buffers[i].view));
	}


}

VkResult ng::graphics::VulkanSwapChain::acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32 * imageIndex)
{
	return fpAcquireNextImageKHR(vulkanDevice->logicalDevice, swapChain, UINT64_MAX, presentCompleteSemaphore, (VkFence)nullptr, imageIndex);
}

VkResult ng::graphics::VulkanSwapChain::queuePresent(VkQueue queue, uint32 imageIndex, VkSemaphore waitSemaphore)
{
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = NULL;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapChain;
	presentInfo.pImageIndices = &imageIndex;

	if (waitSemaphore != VK_NULL_HANDLE) {
		presentInfo.pWaitSemaphores = &waitSemaphore;
		presentInfo.waitSemaphoreCount = 1;
	}
	return fpQueuePresentKHR(queue, &presentInfo);
}

void ng::graphics::VulkanSwapChain::cleanup()
{
	if(swa)
}
