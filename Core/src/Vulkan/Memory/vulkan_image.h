#pragma once
#include "../vulkan_device.h"

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

			virtual void free() = 0;

			static std::size_t hash(VulkanImage const& image);

		};

		typedef VulkanImage VulkanTexture;

		class VulkanTexture2D : VulkanImage {
		private:

		public:

			VulkanTexture2D & operator=(const VulkanTexture2D& other) {
				m_ImageAllocator = other.m_ImageAllocator;
				m_Allocation = other.m_Allocation;
			}

			void moveToDevice() override;

			void freeFromDevice() override;

			void free() override;

		};

		class VulkanTextureArray : VulkanImage {
		private:

		public:

			VulkanTextureArray & operator=(const VulkanTextureArray& other) {
				m_ImageAllocator = other.m_ImageAllocator;
				m_Allocation = other.m_Allocation;
			}

			void moveToDevice() override;

			void freeFromDevice() override;

			void free() override;

		};

	}
}