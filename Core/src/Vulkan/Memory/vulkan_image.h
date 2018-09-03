#pragma once

#include "vulkan_image_allocator.h"

namespace ng {
	namespace vulkan {

		void setImageLayout(
			VkCommandBuffer cmdBuffer,
			VkImage image,
			VkImageLayout oldImageLayout,
			VkImageLayout newImageLayout,
			VkImageSubresourceRange subresourceRnage,
			VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
		);

		void setImageLayout(
			VkCommandBuffer cmdBuffer,
			VkImage image,
			VkImageAspectFlags aspectMask,
			VkImageLayout oldImageLayout,
			VkImageLayout newImageLayout,
			VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

		struct VulkanImageCreateInfo {
			std::string filename;
			VkFormat format;
			VkImageUsageFlags imageUsageFlags;
			VkImageLayout imageLayout;
			bool forceLinear = false;
		};

		class VulkanImageAllocation {
			friend class VulkanImageAllocator;
			friend class VulkanImage;
		private:

		public:

			void updateDescriptor() {
				descriptor.sampler = sampler;
				descriptor.imageView = view;
				descriptor.imageLayout = imageLayout;
			}

			void destroy(VulkanDevice* vulkanDevice) {
				vkDestroyImageView(vulkanDevice->logicalDevice, view, nullptr);
				vkDestroyImage(vulkanDevice->logicalDevice, image, nullptr);
				if (sampler) {
					vkDestroySampler(vulkanDevice->logicalDevice, sampler, nullptr);
				}
				vkFreeMemory(vulkanDevice->logicalDevice, deviceMemory, nullptr);
				if (memSrc != nullptr) {
					free(memSrc);
				}
			}

			bool inDeviceMemory = false;
			VkBool32 forceLinear;
			
			//used if not forcing linear when move to device is needed
			VkBuffer stagingBuffer;
			VkDeviceMemory stagingMemory;
			std::vector<VkBufferImageCopy> bufferCopyRegions;
			//used if forcing linear
			void* memSrc = nullptr;
			uint32 memSize;

			VkImageCreateInfo imageCreateInfo;
			VkImage image;

			VkMemoryAllocateInfo memAllocInfo;
			VkDeviceMemory deviceMemory;
			

			VkImageLayout imageLayout;
			VkImageView view;

			uint32 width, height;
			uint32 mipLevels;
			uint32 layerCount;
			VkDescriptorImageInfo descriptor;

			VkSampler sampler;

		};
		 
		//abstarct
		class VulkanImage {
		private:

		protected:
			friend class VulkanImageAllocator;
			friend class VulkanTexture2D;

			VulkanImageAllocator* m_ImageAllocator;

			//should always exist
			std::shared_ptr<VulkanImageAllocation> m_Allocation = nullptr;
		public:

			virtual void moveToDevice() = 0;

			virtual void freeFromDevice() = 0;

		};

		typedef VulkanImage VulkanTexture;

		class VulkanTexture2D : VulkanImage {
		private:

		public:

			VulkanTexture2D & operator=(const VulkanTexture2D& other) {
				m_ImageAllocator = other.m_ImageAllocator;
				m_Allocation = other.m_Allocation;
			}

			void moveToDevice() override {
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

			void freeFromDevice() override {
				vkDestroyImageView(m_ImageAllocator->m_VulkanDevice->logicalDevice, m_Allocation->view, nullptr);
				vkDestroyImage(m_ImageAllocator->m_VulkanDevice->logicalDevice, m_Allocation->image, nullptr);
				if (m_Allocation->sampler) {
					vkDestroySampler(m_ImageAllocator->m_VulkanDevice->logicalDevice, m_Allocation->sampler, nullptr);
				}
				vkFreeMemory(m_ImageAllocator->m_VulkanDevice->logicalDevice, m_Allocation->deviceMemory, nullptr);
				m_Allocation->inDeviceMemory = false;
			}

			bool free() {
				m_ImageAllocator->freeImage(this);
			}
		};

		class VulkanTextureArray : VulkanImage {
		private:

		public:

			VulkanTextureArray & operator=(const VulkanTextureArray& other) {
				m_ImageAllocator = other.m_ImageAllocator;
				m_Allocation = other.m_Allocation;
			}

			void moveToDevice() override {
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

			void freeFromDevice() override {
				vkDestroyImageView(m_ImageAllocator->m_VulkanDevice->logicalDevice, m_Allocation->view, nullptr);
				vkDestroyImage(m_ImageAllocator->m_VulkanDevice->logicalDevice, m_Allocation->image, nullptr);
				if (m_Allocation->sampler) {
					vkDestroySampler(m_ImageAllocator->m_VulkanDevice->logicalDevice, m_Allocation->sampler, nullptr);
				}
				vkFreeMemory(m_ImageAllocator->m_VulkanDevice->logicalDevice, m_Allocation->deviceMemory, nullptr);
				m_Allocation->inDeviceMemory = false;
			}

			bool free() {
				m_ImageAllocator->freeImage(this);
			}

		};

	}
}