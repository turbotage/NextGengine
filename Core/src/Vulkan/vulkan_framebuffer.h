#pragma once

#include "../def.h"
#include "vulkan_device.h"

namespace ng {
	namespace vulkan {

		struct VulkanFramebufferAttachment {
			VkImage image;
			VkDeviceMemory memory;
			VkImageView view;
			VkFormat format;
			VkImageSubresourceRange subresourceRange;
			VkAttachmentDescription description;

			bool hasDepth();

			bool hasStencil();

			bool isDepthStencil();

		};

		struct VulkanAttachmentCreateInfo {
			uint32 width;
			uint32 height;
			uint32 layerCount;
			VkFormat format;
			VkImageCreateFlags usage;
		};

		struct VulkanFramebufferCreateInfo {
			VulkanDevice* vulkanDevice;
		};

		class VulkanFramebuffer {
		private:

			VulkanDevice* vulkanDevice;

		public:

			uint32 width;
			uint32 height;
			VkFramebuffer framebuffer;
			VkRenderPass renderPass;
			VkSampler sampler;

			std::vector<VulkanFramebufferAttachment> attachments;

		public:

			VulkanFramebuffer();
			VulkanFramebuffer(VulkanFramebufferCreateInfo createInfo);
			~VulkanFramebuffer();

			void init(VulkanFramebufferCreateInfo createInfo);

			uint32 addAttachment(VulkanAttachmentCreateInfo attachmentCreateInfo);

			VkResult createSampler(VkFilter magFilter, VkFilter minFilter, VkSamplerAddressMode addressMode);

			VkResult createRenderPass();

		};

	}
}