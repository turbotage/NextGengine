#include "vulkan_image.h"
#include "vulkan_image_allocator.h"

void ng::vulkan::setImageLayout(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkImageSubresourceRange subresourceRange, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask)
{
	VkImageMemoryBarrier imageMemoryBarrier{};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.oldLayout = oldImageLayout;
	imageMemoryBarrier.newLayout = newImageLayout;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange = subresourceRange;

	switch (oldImageLayout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		// Image layout is undefined (or does not matter)
		// Only valid as initial layout
		// No flags required, listed only for completeness
		imageMemoryBarrier.srcAccessMask = 0;
		break;
	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		// Image is preinitialized
		// Only valid as initial layout for linear images, preserves memory contents
		// Make sure host writes have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		// Image is a color attachment
		// Make sure any writes to the color buffer have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		// Image is a depth/stencil attachment
		// Make sure any writes to the depth/stencil buffer have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		// Image is a transfer source 
		// Make sure any reads from the image have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		// Image is a transfer destination
		// Make sure any writes to the image have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		// Image is read by a shader
		// Make sure any shader reads from the image have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		break;
	}

	// Target layouts (new)
	// Destination access mask controls the dependency for the new image layout
	switch (newImageLayout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		// Image will be used as a transfer destination
		// Make sure any writes to the image have been finished
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		// Image will be used as a transfer source
		// Make sure any reads from the image have been finished
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		// Image will be used as a color attachment
		// Make sure any writes to the color buffer have been finished
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		// Image layout will be used as a depth/stencil attachment
		// Make sure any writes to depth/stencil buffer have been finished
		imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		// Image will be read in a shader (sampler, input attachment)
		// Make sure any writes to the image have been finished
		if (imageMemoryBarrier.srcAccessMask == 0)
		{
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		}
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		// Other source layouts aren't handled (yet)
		break;
	}

	// Put barrier inside setup command buffer
	vkCmdPipelineBarrier(
		cmdBuffer,
		srcStageMask,
		dstStageMask,
		0,
		0, nullptr,
		0, nullptr,
		1, &imageMemoryBarrier);

}

void ng::vulkan::setImageLayout(VkCommandBuffer cmdBuffer, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask)
{
	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = aspectMask;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = 1;
	subresourceRange.layerCount = 1;
	setImageLayout(cmdBuffer, image, oldImageLayout, newImageLayout, subresourceRange, srcStageMask, dstStageMask);
}

/*  VulkanTextureArray  */

void ng::vulkan::VulkanTextureArray::moveToDevice()
{
	if (!m_Allocation->inDeviceMemory) {
		VkMemoryRequirements memReqs;

		VULKAN_CHECK_RESULT(vkCreateImage(m_ImageAllocator->m_VulkanDevice->logicalDevice, &m_Allocation->imageCreateInfo, nullptr, &m_Allocation->image));

		vkGetImageMemoryRequirements(m_ImageAllocator->m_VulkanDevice->logicalDevice, m_Allocation->image, &memReqs);

		m_Allocation->memAllocInfo.allocationSize = memReqs.size;
		m_Allocation->memAllocInfo.memoryTypeIndex = m_ImageAllocator->m_VulkanDevice->getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VULKAN_CHECK_RESULT(vkAllocateMemory(m_ImageAllocator->m_VulkanDevice->logicalDevice, &m_Allocation->memAllocInfo, nullptr, &m_Allocation->deviceMemory));
		VULKAN_CHECK_RESULT(vkBindImageMemory(m_ImageAllocator->m_VulkanDevice->logicalDevice, m_Allocation->image, m_Allocation->deviceMemory, 0));

		VkCommandBuffer copyCmd = m_ImageAllocator->m_VulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_ImageAllocator->m_VulkanDevice->memoryCommandPool, true);

		// Image barrier for optimal image (target)
		// Set initial layout for all array layers (faces) of the optimal (target) tiled texture
		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.layerCount = m_Allocation->layerCount;

		setImageLayout(
			copyCmd,
			m_Allocation->image,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			subresourceRange);

		// Copy the cube map faces from the staging buffer to the optimal tiled image
		vkCmdCopyBufferToImage(
			copyCmd,
			m_Allocation->stagingBuffer,
			m_Allocation->image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			m_Allocation->bufferCopyRegions.size(),
			m_Allocation->bufferCopyRegions.data()
		);

		// Change texture image layout to shader read after all faces have been copied
		m_Allocation->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		setImageLayout(
			copyCmd,
			m_Allocation->image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			m_Allocation->imageLayout,
			subresourceRange);

		m_ImageAllocator->m_VulkanDevice->flushCommandBuffer(copyCmd, m_ImageAllocator->m_VulkanDevice->memoryCommandPool, m_ImageAllocator->m_VulkanDevice->transferQueue, true);

		// Create sampler
		VkSamplerCreateInfo samplerCreateInfo = {};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCreateInfo.addressModeV = samplerCreateInfo.addressModeU;
		samplerCreateInfo.addressModeW = samplerCreateInfo.addressModeU;
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.maxAnisotropy = 8;
		samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = 0.0f;
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		VULKAN_CHECK_RESULT(vkCreateSampler(m_ImageAllocator->m_VulkanDevice->logicalDevice, &samplerCreateInfo, nullptr, &m_Allocation->sampler));

		// Create image view
		VkImageViewCreateInfo viewCreateInfo = {};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		viewCreateInfo.format = m_Allocation->imageCreateInfo.format;
		viewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		viewCreateInfo.subresourceRange.layerCount = m_Allocation->layerCount;
		viewCreateInfo.subresourceRange.levelCount = 1;
		viewCreateInfo.image = m_Allocation->image;
		VULKAN_CHECK_RESULT(vkCreateImageView(m_ImageAllocator->m_VulkanDevice->logicalDevice, &viewCreateInfo, nullptr, &m_Allocation->view));

		m_Allocation->inDeviceMemory = true;
	}
}

void ng::vulkan::VulkanTextureArray::freeFromDevice()
{
	vkDestroyImageView(m_ImageAllocator->m_VulkanDevice->logicalDevice, m_Allocation->view, nullptr);
	vkDestroyImage(m_ImageAllocator->m_VulkanDevice->logicalDevice, m_Allocation->image, nullptr);
	if (m_Allocation->sampler) {
		vkDestroySampler(m_ImageAllocator->m_VulkanDevice->logicalDevice, m_Allocation->sampler, nullptr);
	}
	vkFreeMemory(m_ImageAllocator->m_VulkanDevice->logicalDevice, m_Allocation->deviceMemory, nullptr);
	m_Allocation->inDeviceMemory = false;
}

void ng::vulkan::VulkanTextureArray::free()
{
	m_ImageAllocator->freeImage(this);
}

/*  VulkanTexture2D  */

void ng::vulkan::VulkanTexture2D::moveToDevice()
{
	if (!m_Allocation->inDeviceMemory) {
		VkMemoryRequirements memReqs;

		VkCommandBuffer copyCmd =
			m_ImageAllocator->m_VulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_ImageAllocator->m_VulkanDevice->memoryCommandPool, true);

		if (!m_Allocation->forceLinear) {

			VULKAN_CHECK_RESULT(vkCreateImage(m_ImageAllocator->m_VulkanDevice->logicalDevice, &m_Allocation->imageCreateInfo, nullptr, &m_Allocation->image));

			vkGetImageMemoryRequirements(m_ImageAllocator->m_VulkanDevice->logicalDevice, m_Allocation->image, &memReqs);

			m_Allocation->memAllocInfo.allocationSize = memReqs.size;

			m_Allocation->memAllocInfo.memoryTypeIndex = m_ImageAllocator->m_VulkanDevice->getMemoryTypeIndex(
				memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			VULKAN_CHECK_RESULT(
				vkAllocateMemory(m_ImageAllocator->m_VulkanDevice->logicalDevice, &m_Allocation->memAllocInfo, nullptr, &m_Allocation->deviceMemory));
			VULKAN_CHECK_RESULT(
				vkBindImageMemory(m_ImageAllocator->m_VulkanDevice->logicalDevice, m_Allocation->image, m_Allocation->deviceMemory, 0));

			VkImageSubresourceRange subresourceRange = {};
			subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.levelCount = m_Allocation->mipLevels;
			subresourceRange.layerCount = 1;

			setImageLayout(
				copyCmd,
				m_Allocation->image,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				subresourceRange
			);

			vkCmdCopyBufferToImage(
				copyCmd,
				m_Allocation->stagingBuffer,
				m_Allocation->image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				static_cast<uint32>(m_Allocation->bufferCopyRegions.size()),
				m_Allocation->bufferCopyRegions.data()
			);

			//image-layout should be last as the same
			setImageLayout(
				copyCmd,
				m_Allocation->image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				m_Allocation->imageLayout,
				subresourceRange
			);

			m_ImageAllocator->m_VulkanDevice->flushCommandBuffer(copyCmd, m_ImageAllocator->m_VulkanDevice->memoryCommandPool, m_ImageAllocator->m_VulkanDevice->transferQueue);
		}
		else {
			// Load mip map level 0 to linear tiling image
			VULKAN_CHECK_RESULT(vkCreateImage(m_ImageAllocator->m_VulkanDevice->logicalDevice, &m_Allocation->imageCreateInfo, nullptr, &m_Allocation->image));

			// Get memory requirements for this image 
			// like size and alignment
			vkGetImageMemoryRequirements(m_ImageAllocator->m_VulkanDevice->logicalDevice, m_Allocation->image, &memReqs);
			// Set memory allocation size to required memory size
			m_Allocation->memAllocInfo.allocationSize = memReqs.size;

			// Get memory type that can be mapped to host memory
			m_Allocation->memAllocInfo.memoryTypeIndex = m_ImageAllocator->m_VulkanDevice->getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			// Allocate host memory
			VULKAN_CHECK_RESULT(vkAllocateMemory(m_ImageAllocator->m_VulkanDevice->logicalDevice, &m_Allocation->memAllocInfo, nullptr, &m_Allocation->deviceMemory));

			// Bind allocated image for use
			VULKAN_CHECK_RESULT(vkBindImageMemory(m_ImageAllocator->m_VulkanDevice->logicalDevice, m_Allocation->image, m_Allocation->deviceMemory, 0));

			// Get sub resource layout
			// Mip map count, array layer, etc.
			VkImageSubresource subRes = {};
			subRes.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subRes.mipLevel = 0;

			VkSubresourceLayout subResLayout;
			void *data;

			// Get sub resources layout 
			// Includes row pitch, size offsets, etc.
			vkGetImageSubresourceLayout(m_ImageAllocator->m_VulkanDevice->logicalDevice, m_Allocation->image, &subRes, &subResLayout);

			// Map image memory
			VULKAN_CHECK_RESULT(vkMapMemory(m_ImageAllocator->m_VulkanDevice->logicalDevice, m_Allocation->deviceMemory, 0, memReqs.size, 0, &data));

			// Copy image data into memory
			memcpy(data, m_Allocation->memSrc, m_Allocation->memSize);

			vkUnmapMemory(m_ImageAllocator->m_VulkanDevice->logicalDevice, m_Allocation->deviceMemory);

			//image layout should be same as last

			// Setup image memory barrier
			setImageLayout(
				copyCmd,
				m_Allocation->image,
				VK_IMAGE_ASPECT_COLOR_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED,
				m_Allocation->imageLayout
			);

			m_ImageAllocator->m_VulkanDevice->flushCommandBuffer(copyCmd, m_ImageAllocator->m_VulkanDevice->memoryCommandPool, m_ImageAllocator->m_VulkanDevice->transferQueue);
		}

		// Create a defaultsampler
		VkSamplerCreateInfo samplerCreateInfo = {};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
		samplerCreateInfo.minLod = 0.0f;
		// Max level-of-detail should match mip level count
		samplerCreateInfo.maxLod = (!m_Allocation->forceLinear) ? (float)m_Allocation->mipLevels : 0.0f;
		// Only enable anisotropic filtering if enabled on the devicec
		samplerCreateInfo.maxAnisotropy = m_ImageAllocator->m_VulkanDevice->enabledFeatures.samplerAnisotropy ? m_ImageAllocator->m_VulkanDevice->properties.limits.maxSamplerAnisotropy : 1.0f;
		samplerCreateInfo.anisotropyEnable = m_ImageAllocator->m_VulkanDevice->enabledFeatures.samplerAnisotropy;
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		VULKAN_CHECK_RESULT(vkCreateSampler(m_ImageAllocator->m_VulkanDevice->logicalDevice, &samplerCreateInfo, nullptr, &m_Allocation->sampler));

		// Create image view
		// Textures are not directly accessed by the shaders and
		// are abstracted by image views containing additional
		// information and sub resource ranges
		VkImageViewCreateInfo viewCreateInfo = {};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.format = m_Allocation->imageCreateInfo.format;
		viewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		// Linear tiling usually won't support mip maps
		// Only set mip map count if optimal tiling is used
		viewCreateInfo.subresourceRange.levelCount = (!m_Allocation->forceLinear) ? m_Allocation->mipLevels : 1;
		viewCreateInfo.image = m_Allocation->image;
		VULKAN_CHECK_RESULT(vkCreateImageView(m_ImageAllocator->m_VulkanDevice->logicalDevice, &viewCreateInfo, nullptr, &m_Allocation->view));

		// Update descriptor image info member that can be used for setting up descriptor sets
		m_Allocation->updateDescriptor();

		m_Allocation->inDeviceMemory = true;
	}
}

void ng::vulkan::VulkanTexture2D::freeFromDevice()
{
	vkDestroyImageView(m_ImageAllocator->m_VulkanDevice->logicalDevice, m_Allocation->view, nullptr);
	vkDestroyImage(m_ImageAllocator->m_VulkanDevice->logicalDevice, m_Allocation->image, nullptr);
	if (m_Allocation->sampler) {
		vkDestroySampler(m_ImageAllocator->m_VulkanDevice->logicalDevice, m_Allocation->sampler, nullptr);
	}
	vkFreeMemory(m_ImageAllocator->m_VulkanDevice->logicalDevice, m_Allocation->deviceMemory, nullptr);
	m_Allocation->inDeviceMemory = false;
}

void ng::vulkan::VulkanTexture2D::free()
{
	m_ImageAllocator->freeImage(this);
}

std::size_t ng::vulkan::VulkanImage::hash(VulkanImage const & image)
{
	return (size_t)image.m_Allocation.get();
}
