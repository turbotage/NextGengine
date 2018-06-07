#include "vulkan_framebuffer.h"
#include "../debug.h"


bool ng::vulkan::VulkanFramebufferAttachment::hasDepth()
{
	std::vector<VkFormat> formats = {
		VK_FORMAT_D16_UNORM,
		VK_FORMAT_X8_D24_UNORM_PACK32,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D32_SFLOAT_S8_UINT
	};
	return std::find(formats.begin(), formats.end(), format) != std::end(formats);
}

bool ng::vulkan::VulkanFramebufferAttachment::hasStencil()
{
	std::vector<VkFormat> formats = {
		VK_FORMAT_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT
	};
	return std::find(formats.begin(), formats.end(), format) != std::end(formats);
}

bool ng::vulkan::VulkanFramebufferAttachment::isDepthStencil()
{
	return(hasDepth() || hasStencil());
}

ng::vulkan::VulkanFramebuffer::VulkanFramebuffer(VulkanDevice * vulkanDevice)
{
	assert(vulkanDevice);
	this->vulkanDevice = vulkanDevice;
}

ng::vulkan::VulkanFramebuffer::~VulkanFramebuffer()
{
	assert(vulkanDevice);
	for (auto attachment : attachments) {
		vkDestroyImage(vulkanDevice->logicalDevice, attachment.image, nullptr);
		vkDestroyImageView(vulkanDevice->logicalDevice, attachment.view, nullptr);
		vkFreeMemory(vulkanDevice->logicalDevice, attachment.memory, nullptr);
	}
	vkDestroySampler(vulkanDevice->logicalDevice, sampler, nullptr);
	vkDestroyRenderPass(vulkanDevice->logicalDevice, renderPass, nullptr);
	vkDestroyFramebuffer(vulkanDevice->logicalDevice, framebuffer, nullptr);
}

uint32 ng::vulkan::VulkanFramebuffer::addAttachment(VulkanAttachmentCreateInfo attachmentCreateInfo)
{
	VulkanFramebufferAttachment attachment;
	attachment.format = attachmentCreateInfo.format;

	VkImageAspectFlags aspectMask = 0;

	if (attachmentCreateInfo.usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
		aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	if (attachmentCreateInfo.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
		aspectMask = aspectMask | VK_IMAGE_ASPECT_STENCIL_BIT;
	}

	assert(aspectMask > 0);

	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	imageCreateInfo.format = attachmentCreateInfo.format;
	imageCreateInfo.extent.width = attachmentCreateInfo.width;
	imageCreateInfo.extent.height = attachmentCreateInfo.height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = attachmentCreateInfo.layerCount;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = attachmentCreateInfo.usage;

	VkMemoryAllocateInfo memAllocInfo = {};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;

	VkMemoryRequirements memReqs;

	VULKAN_CHECK_RESULT(vkCreateImage(vulkanDevice->logicalDevice, &imageCreateInfo, nullptr, &attachment.image));

	vkGetImageMemoryRequirements(vulkanDevice->logicalDevice, attachment.image, &memReqs);

	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = vulkanDevice->getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VULKAN_CHECK_RESULT(vkAllocateMemory(vulkanDevice->logicalDevice, &memAllocInfo, nullptr, &attachment.memory));
	VULKAN_CHECK_RESULT(vkBindImageMemory(vulkanDevice->logicalDevice, attachment.image, attachment.memory, 0));

	attachment.subresourceRange = {};
	attachment.subresourceRange.aspectMask = aspectMask;
	attachment.subresourceRange.levelCount = 1;
	attachment.subresourceRange.layerCount = attachmentCreateInfo.layerCount;

	VkImageViewCreateInfo imageView = {};
	imageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageView.viewType = (attachmentCreateInfo.layerCount == 1) ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY;
	imageView.format = attachmentCreateInfo.format;
	imageView.subresourceRange = attachment.subresourceRange;

	imageView.subresourceRange.aspectMask = (attachment.hasDepth()) ? VK_IMAGE_ASPECT_DEPTH_BIT : aspectMask;
	imageView.image = attachment.image;

	VULKAN_CHECK_RESULT(vkCreateImageView(vulkanDevice->logicalDevice, &imageView, nullptr, &attachment.view));

	attachment.description = {};
	attachment.description.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachment.description.storeOp = (attachmentCreateInfo.usage & VK_IMAGE_USAGE_SAMPLED_BIT) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.description.format = attachmentCreateInfo.format;
	attachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	if (attachment.hasDepth() || attachment.hasStencil())
	{
		attachment.description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	}
	else
	{
		attachment.description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

	attachments.push_back(attachment);

	return static_cast<uint32_t>(attachments.size() - 1);

}

VkResult ng::vulkan::VulkanFramebuffer::createSampler(VkFilter magFilter, VkFilter minFilter, VkSamplerAddressMode addressMode)
{
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.maxAnisotropy = 1.0f;
	samplerInfo.magFilter = magFilter;
	samplerInfo.minFilter = minFilter;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.addressModeU = addressMode;
	samplerInfo.addressModeV = addressMode;
	samplerInfo.addressModeW = addressMode;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.maxAnisotropy = 1.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 1.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	return vkCreateSampler(vulkanDevice->logicalDevice, &samplerInfo, nullptr, &sampler);
}

VkResult ng::vulkan::VulkanFramebuffer::createRenderPass()
{
	std::vector<VkAttachmentDescription> attachmentDescriptions;
	for (auto& attachment : attachments) {
		attachmentDescriptions.push_back(attachment.description);
	}

	std::vector<VkAttachmentReference> colorReferences;
	VkAttachmentReference depthReference;
	bool hasDepth = false;
	bool hasColor = false;

	uint32 attachmentIndex = 0;

	for (auto& attachment : attachments) {
		if (attachment.isDepthStencil()) {
			assert(!hasDepth);
			depthReference.attachment = attachmentIndex;
			depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			hasDepth = true;
		}
		else {
			colorReferences.push_back({ attachmentIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
			hasColor = true;
		}
		attachmentIndex++;
	}

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	if (hasColor) {
		subpass.pColorAttachments = colorReferences.data();
		subpass.colorAttachmentCount = static_cast<uint32>(colorReferences.size());
	}
	if (hasDepth) {
		subpass.pDepthStencilAttachment = &depthReference;
	}

	std::array<VkSubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	// Create render pass
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pAttachments = attachmentDescriptions.data();
	renderPassInfo.attachmentCount = static_cast<uint32>(attachmentDescriptions.size());
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = dependencies.data();
	VULKAN_CHECK_RESULT(vkCreateRenderPass(vulkanDevice->logicalDevice, &renderPassInfo, nullptr, &renderPass));

	std::vector<VkImageView> attachmentViews;
	for (auto attachment : attachments) {
		attachmentViews.push_back(attachment.view);
	}

	uint32 maxLayers = 0;
	for (auto attachment : attachments) {
		if (attachment.subresourceRange.layerCount > maxLayers) {
			maxLayers = attachment.subresourceRange.layerCount;
		}
	}

	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = renderPass;
	framebufferInfo.pAttachments = attachmentViews.data();
	framebufferInfo.attachmentCount = static_cast<uint32>(attachmentViews.size());
	framebufferInfo.width = width;
	framebufferInfo.height = height;
	framebufferInfo.layers = maxLayers;
	
	VULKAN_CHECK_RESULT(vkCreateFramebuffer(vulkanDevice->logicalDevice, &framebufferInfo, nullptr, &framebuffer));

	return VK_SUCCESS;
}


