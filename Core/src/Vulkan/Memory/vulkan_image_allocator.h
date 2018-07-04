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

		public:

			VulkanImageAllocator(VulkanDevice* vulkanDevice, VkMemoryPropertyFlags flags, VkMemoryAlignment alignment, VkDeviceSize standardAllocSize) {
				m_VulkanDevice = vulkanDevice;
				m_MemoryFlags = flags;
				m_MemoryAlignment = alignment;
				m_StandardChunkSize = standardAllocSize;
			}

		};

	}
}