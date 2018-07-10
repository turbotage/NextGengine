#pragma once

#include "../../def.h"
#include "vulkan_image.h"
#include "vulkan_memory_chunk.h"

namespace ng {
	namespace vulkan {

		class VulkanImageAllocator {
		private:
			VulkanDevice* m_VulkanDevice;

			VkDeviceSize m_StandardChunkSize;
			VkMemoryPropertyFlags m_MemoryFlags;
			VkMemoryAlignment m_MemoryAlignment;
			VkBufferUsageFlags m_BufferUsage;

			/*  When buffers should reside in device-local memory when used this are the vulkan-memory-chunks that holds the buffers for staging,
			the data will always be available here in the staging chunks to enable fast swapping. If the buffers should be non device-local, perhaps */
			std::forward_list<VulkanImageChunk> m_StagingChunks;

			/*  The vulkan-memory-chunks holding the buffers when they reside in device-local memory,
			never used for host visible only buffers  */
			std::forward_list<VulkanImageChunk> m_DeviceChunks;

			std::forward_list<VulkanImageChunk>::iterator addChunk(std::forward_list<VulkanImageChunk>* chunks, VkResult* result = nullptr);

			VkDeviceSize getAlignedSize(VkDeviceSize size);

		public:
			
			VulkanImageAllocator(VulkanDevice* vulkanDevice, VkMemoryPropertyFlags flags, VkMemoryAlignment alignment, VkDeviceSize standardAllocSize);

			VulkanImageAllocator(const VulkanImageAllocator& other) = delete;

			VulkanImageAllocator(VulkanBuffer &&) = delete;

			void createImage(VulkanImageCreateInfo createInfo, VulkanImage* image);

			void defragmentDeviceMem(uint16 chunksDefragNum = UINT16_MAX, bool waitUntilComplete = true);

			void defragmentStagingMem(uint16 chunksDefragNum = UINT16_MAX, bool waitUntilComplete = true);

		};

	}
}