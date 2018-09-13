#include "vulkan_image_allocator.h"
#include "vulkan_image.h"
#include <gli/gli.hpp>

ng::vulkan::VulkanImageAllocator::VulkanImageAllocator()
{
}

ng::vulkan::VulkanImageAllocator::VulkanImageAllocator(VulkanImageAllocatorCreateInfo createInfo)
{
	create(createInfo);
}

void ng::vulkan::VulkanImageAllocator::create(VulkanImageAllocatorCreateInfo createInfo)
{
	m_VulkanDevice = createInfo.vulkanDevice;
}

void ng::vulkan::VulkanImageAllocator::createTexture2D(VulkanImageCreateInfo createInfo, VulkanTexture2D * image)
{

	VulkanImageAllocation imageAllocation;

	if (!tools::fileExists(createInfo.filename)) {
		LOGD("the file didn't exist");
	}

	gli::texture2d tex2D(gli::load(createInfo.filename.c_str()));

	assert(!tex2D.empty());

	this->m_VulkanDevice = m_VulkanDevice;
	imageAllocation.width = static_cast<uint32>(tex2D[0].extent().x);
	imageAllocation.height = static_cast<uint32>(tex2D[0].extent().y);
	imageAllocation.mipLevels = static_cast<uint32>(tex2D.levels());

	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(m_VulkanDevice->physicalDevice, createInfo.format, &formatProperties);

	imageAllocation.forceLinear = createInfo.forceLinear;

	imageAllocation.memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements memReqs;

	VkCommandBuffer copyCmd =
		m_VulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_VulkanDevice->memoryCommandPool, true);

	if (!imageAllocation.forceLinear) {

		m_VulkanDevice->createBuffer(
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			tex2D.size(),
			&imageAllocation.stagingBuffer,
			&imageAllocation.stagingMemory,
			tex2D.data()
		);

		uint32 offset = 0;

		for (uint32 i = 0; i < imageAllocation.mipLevels; ++i) {
			VkBufferImageCopy bufferCopyRegion = {};
			bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			bufferCopyRegion.imageSubresource.mipLevel = i;
			bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
			bufferCopyRegion.imageSubresource.layerCount = 1;
			bufferCopyRegion.imageExtent.width = static_cast<uint32>(tex2D[i].extent().x);
			bufferCopyRegion.imageExtent.height = static_cast<uint32>(tex2D[i].extent().y);
			bufferCopyRegion.imageExtent.depth = 1;
			bufferCopyRegion.bufferOffset = offset;

			imageAllocation.bufferCopyRegions.push_back(bufferCopyRegion);

			offset += static_cast<uint32>(tex2D[i].size());

		}

		imageAllocation.imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageAllocation.imageCreateInfo.format = createInfo.format;
		imageAllocation.imageCreateInfo.mipLevels = imageAllocation.mipLevels;
		imageAllocation.imageCreateInfo.arrayLayers = 1;
		imageAllocation.imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageAllocation.imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageAllocation.imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageAllocation.imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageAllocation.imageCreateInfo.extent = { imageAllocation.width, imageAllocation.height, 1 };
		imageAllocation.imageCreateInfo.usage = createInfo.imageUsageFlags;

		if (!(imageAllocation.imageCreateInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
			imageAllocation.imageCreateInfo.usage != VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}

		VULKAN_CHECK_RESULT(vkCreateImage(m_VulkanDevice->logicalDevice, &imageAllocation.imageCreateInfo, nullptr, &imageAllocation.image));

		vkGetImageMemoryRequirements(m_VulkanDevice->logicalDevice, imageAllocation.image, &memReqs);

		imageAllocation.memAllocInfo.allocationSize = memReqs.size;

		imageAllocation.memAllocInfo.memoryTypeIndex = m_VulkanDevice->getMemoryTypeIndex(
			memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VULKAN_CHECK_RESULT(
			vkAllocateMemory(m_VulkanDevice->logicalDevice, &imageAllocation.memAllocInfo, nullptr, &imageAllocation.deviceMemory));
		VULKAN_CHECK_RESULT(
			vkBindImageMemory(m_VulkanDevice->logicalDevice, imageAllocation.image, imageAllocation.deviceMemory, 0));

		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = imageAllocation.mipLevels;
		subresourceRange.layerCount = 1;

		setImageLayout(
			copyCmd,
			imageAllocation.image,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			subresourceRange
		);

		vkCmdCopyBufferToImage(
			copyCmd,
			imageAllocation.stagingBuffer,
			imageAllocation.image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			static_cast<uint32>(imageAllocation.bufferCopyRegions.size()),
			imageAllocation.bufferCopyRegions.data()
		);

		imageAllocation.imageLayout = createInfo.imageLayout;
		setImageLayout(
			copyCmd,
			imageAllocation.image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			createInfo.imageLayout,
			subresourceRange
		);

		m_VulkanDevice->flushCommandBuffer(copyCmd, m_VulkanDevice->memoryCommandPool, m_VulkanDevice->transferQueue);

	}
	else {
		// Prefer using optimal tiling, as linear tiling 
		// may support only a small set of features 
		// depending on implementation (e.g. no mip maps, only one layer, etc.)

		// Check if this support is supported for linear tiling
		assert(formatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);

		imageAllocation.imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageAllocation.imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageAllocation.imageCreateInfo.format = createInfo.format;
		imageAllocation.imageCreateInfo.extent = { imageAllocation.width, imageAllocation.height, 1 };
		imageAllocation.imageCreateInfo.mipLevels = 1;
		imageAllocation.imageCreateInfo.arrayLayers = 1;
		imageAllocation.imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageAllocation.imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
		imageAllocation.imageCreateInfo.usage = createInfo.imageUsageFlags;
		imageAllocation.imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageAllocation.imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		// Load mip map level 0 to linear tiling image
		VULKAN_CHECK_RESULT(vkCreateImage(m_VulkanDevice->logicalDevice, &imageAllocation.imageCreateInfo, nullptr, &imageAllocation.image));

		// Get memory requirements for this image 
		// like size and alignment
		vkGetImageMemoryRequirements(m_VulkanDevice->logicalDevice, imageAllocation.image, &memReqs);
		// Set memory allocation size to required memory size
		imageAllocation.memAllocInfo.allocationSize = memReqs.size;

		// Get memory type that can be mapped to host memory
		imageAllocation.memAllocInfo.memoryTypeIndex = m_VulkanDevice->getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		// Allocate host memory
		VULKAN_CHECK_RESULT(vkAllocateMemory(m_VulkanDevice->logicalDevice, &imageAllocation.memAllocInfo, nullptr, &imageAllocation.deviceMemory));

		// Bind allocated image for use
		VULKAN_CHECK_RESULT(vkBindImageMemory(m_VulkanDevice->logicalDevice, imageAllocation.image, imageAllocation.deviceMemory, 0));

		// Get sub resource layout
		// Mip map count, array layer, etc.
		VkImageSubresource subRes = {};
		subRes.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subRes.mipLevel = 0;

		VkSubresourceLayout subResLayout;
		void *data;

		// Get sub resources layout 
		// Includes row pitch, size offsets, etc.
		vkGetImageSubresourceLayout(m_VulkanDevice->logicalDevice, imageAllocation.image, &subRes, &subResLayout);

		imageAllocation.memSize = tex2D[subRes.mipLevel].size();
		imageAllocation.memSrc = malloc(imageAllocation.memSize);

		// Map image memory
		VULKAN_CHECK_RESULT(vkMapMemory(m_VulkanDevice->logicalDevice, imageAllocation.deviceMemory, 0, memReqs.size, 0, &data));

		// Copy image data into memory
		memcpy(data, imageAllocation.memSrc, imageAllocation.memSize);

		vkUnmapMemory(m_VulkanDevice->logicalDevice, imageAllocation.deviceMemory);

		imageAllocation.imageLayout = createInfo.imageLayout;

		// Setup image memory barrier
		setImageLayout(
			copyCmd,
			imageAllocation.image,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			createInfo.imageLayout
		);

		m_VulkanDevice->flushCommandBuffer(copyCmd, m_VulkanDevice->memoryCommandPool, m_VulkanDevice->transferQueue);
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
	samplerCreateInfo.maxLod = (!imageAllocation.forceLinear) ? (float)imageAllocation.mipLevels : 0.0f;
	// Only enable anisotropic filtering if enabled on the devicec
	samplerCreateInfo.maxAnisotropy = m_VulkanDevice->enabledFeatures.samplerAnisotropy ? m_VulkanDevice->properties.limits.maxSamplerAnisotropy : 1.0f;
	samplerCreateInfo.anisotropyEnable = m_VulkanDevice->enabledFeatures.samplerAnisotropy;
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	VULKAN_CHECK_RESULT(vkCreateSampler(m_VulkanDevice->logicalDevice, &samplerCreateInfo, nullptr, &imageAllocation.sampler));

	// Create image view
	// Textures are not directly accessed by the shaders and
	// are abstracted by image views containing additional
	// information and sub resource ranges
	VkImageViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.format = createInfo.format;
	viewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	// Linear tiling usually won't support mip maps
	// Only set mip map count if optimal tiling is used
	viewCreateInfo.subresourceRange.levelCount = (!imageAllocation.forceLinear) ? imageAllocation.mipLevels : 1;
	viewCreateInfo.image = imageAllocation.image;
	VULKAN_CHECK_RESULT(vkCreateImageView(m_VulkanDevice->logicalDevice, &viewCreateInfo, nullptr, &imageAllocation.view));

	// Update descriptor image info member that can be used for setting up descriptor sets
	imageAllocation.updateDescriptor();

	m_Allocations.push_front(imageAllocation);
	image->m_Allocation = std::shared_ptr<VulkanImageAllocation>(&m_Allocations.front());
	image->m_ImageAllocator = this;
}

void ng::vulkan::VulkanImageAllocator::createTextureArray(VulkanImageCreateInfo createInfo, VulkanTextureArray * image)
{
	VulkanImageAllocation imageAllocation;

	if (m_VulkanDevice->features.textureCompressionBC) {
		createInfo.filename = createInfo.filename + "_bc3_unorm.ktx";
		createInfo.format = VK_FORMAT_BC3_UNORM_BLOCK;
	}
	else if (m_VulkanDevice->features.textureCompressionASTC_LDR) {
		createInfo.filename = createInfo.filename + "_astc_8x8_unorm.ktx";
		createInfo.format = VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
	}
	else if (m_VulkanDevice->features.textureCompressionETC2) {
		createInfo.filename = createInfo.filename + "_etc2_unorm.ktx";
		createInfo.format = VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
	}
	else {
		tools::exitFatal("Device does not support any compressed texture format!", VK_ERROR_FEATURE_NOT_PRESENT);
	}

	if (!tools::fileExists(createInfo.filename)) {
		LOGD("the file didn't exist");
	}

	gli::texture2d_array tex2DArray(gli::load(createInfo.filename));

	assert(!tex2DArray.empty());

	imageAllocation.width = tex2DArray.extent().x;
	imageAllocation.height = tex2DArray.extent().y;
	imageAllocation.layerCount = tex2DArray.layers();

	imageAllocation.memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements memReqs;

	VkCommandBuffer copyCmd =
		m_VulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_VulkanDevice->memoryCommandPool, true);

	m_VulkanDevice->createBuffer(
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		tex2DArray.size(),
		&imageAllocation.stagingBuffer,
		&imageAllocation.stagingMemory,
		tex2DArray.data()
	);

	size_t offset = 0;

	for (uint32 layer = 0; layer < image->m_Allocation->layerCount; ++layer) {
		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel = 0;
		bufferCopyRegion.imageSubresource.baseArrayLayer = layer;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = static_cast<uint32_t>(tex2DArray[layer][0].extent().x);
		bufferCopyRegion.imageExtent.height = static_cast<uint32_t>(tex2DArray[layer][0].extent().y);
		bufferCopyRegion.imageExtent.depth = 1;
		bufferCopyRegion.bufferOffset = offset;

		imageAllocation.bufferCopyRegions.push_back(bufferCopyRegion);

		offset += tex2DArray[layer][0].size();
	}

	imageAllocation.imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageAllocation.imageCreateInfo.format = createInfo.format;
	imageAllocation.imageCreateInfo.mipLevels = 1;
	imageAllocation.imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageAllocation.imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageAllocation.imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageAllocation.imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageAllocation.imageCreateInfo.extent = { imageAllocation.width, imageAllocation.height, 1 };
	imageAllocation.imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageAllocation.imageCreateInfo.arrayLayers = imageAllocation.layerCount;

	VULKAN_CHECK_RESULT(vkCreateImage(m_VulkanDevice->logicalDevice, &imageAllocation.imageCreateInfo, nullptr, &imageAllocation.image));

	vkGetImageMemoryRequirements(m_VulkanDevice->logicalDevice, imageAllocation.image, &memReqs);

	imageAllocation.memAllocInfo.allocationSize = memReqs.size;
	imageAllocation.memAllocInfo.memoryTypeIndex = m_VulkanDevice->getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VULKAN_CHECK_RESULT(vkAllocateMemory(m_VulkanDevice->logicalDevice, &imageAllocation.memAllocInfo, nullptr, &imageAllocation.deviceMemory));
	VULKAN_CHECK_RESULT(vkBindImageMemory(m_VulkanDevice->logicalDevice, imageAllocation.image, imageAllocation.deviceMemory, 0));

	VkCommandBuffer copyCmd = m_VulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_VulkanDevice->memoryCommandPool,true);

	// Image barrier for optimal image (target)
	// Set initial layout for all array layers (faces) of the optimal (target) tiled texture
	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = 1;
	subresourceRange.layerCount = imageAllocation.layerCount;

	setImageLayout(
		copyCmd,
		imageAllocation.image,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		subresourceRange);

	// Copy the cube map faces from the staging buffer to the optimal tiled image
	vkCmdCopyBufferToImage(
		copyCmd,
		imageAllocation.stagingBuffer,
		imageAllocation.image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		imageAllocation.bufferCopyRegions.size(),
		imageAllocation.bufferCopyRegions.data()
	);

	// Change texture image layout to shader read after all faces have been copied
	imageAllocation.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	setImageLayout(
		copyCmd,
		imageAllocation.image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		imageAllocation.imageLayout,
		subresourceRange);

	m_VulkanDevice->flushCommandBuffer(copyCmd, m_VulkanDevice->memoryCommandPool, m_VulkanDevice->transferQueue, true);

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
	VULKAN_CHECK_RESULT(vkCreateSampler(m_VulkanDevice->logicalDevice, &samplerCreateInfo, nullptr, &imageAllocation.sampler));

	// Create image view
	VkImageViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
	viewCreateInfo.format = createInfo.format;
	viewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	viewCreateInfo.subresourceRange.layerCount = imageAllocation.layerCount;
	viewCreateInfo.subresourceRange.levelCount = 1;
	viewCreateInfo.image = imageAllocation.image;
	VULKAN_CHECK_RESULT(vkCreateImageView(m_VulkanDevice->logicalDevice, &viewCreateInfo, nullptr, &imageAllocation.view));

	m_Allocations.push_front(imageAllocation);
	image->m_Allocation = std::shared_ptr<VulkanImageAllocation>(&m_Allocations.front());
	image->m_ImageAllocator = this;

}

void ng::vulkan::VulkanImageAllocator::freeImage(VulkanImage * image)
{
	if (image->m_ImageAllocator != this) {
		LOGD("tried to free image that doesn't belong to this allocator");
	}
	if (image->m_Allocation != nullptr) {
		if (image->m_Allocation.use_count() == 1) {
			image->m_Allocation->destroy(m_VulkanDevice);
		}
	}
}


