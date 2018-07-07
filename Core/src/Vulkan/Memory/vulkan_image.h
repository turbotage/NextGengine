#pragma once

#include "vulkan_image_allocator.h"
#include "vulkan_memory_chunk.h"

namespace ng {
	namespace vulkan {

		struct VulkanImageCreateInfo {
			VkDeviceSize size;
			void* data;
		};

		class VulkanImage {
		private:
			friend class VulkanBufferAllocator;
			VulkanImageAllocator* m_ImageAllocator;

			//should always exist
			VkImage m_StagingImage = VK_NULL_HANDLE;
			VulkanBufferChunk* m_StagingChunk = nullptr;
			std::shared_ptr<VulkanAllocation> m_StagingAllocation = nullptr;

			VkImage m_DeviceImage = VK_NULL_HANDLE;
			VulkanBufferChunk* m_DeviceChunk = nullptr;
			std::shared_ptr<VulkanAllocation> m_DeviceAllocation = nullptr;

		public:
			/* will return false if this is host local memory only */
			bool moveToDevice() {
				auto it = m_DeviceChunk->getClosestAllocationMatch(m_StagingAllocation->size);
			}

			bool free() {
				if (m_DeviceAllocation != nullptr) {
					if (m_DeviceAllocation.use_count() == 1) {
						m_DeviceChunk->free(*m_DeviceAllocation);
					}
				}
				m_DeviceAllocation.reset();
				if (m_StagingAllocation.use_count() == 1) {
					m_StagingChunk->free(*m_StagingAllocation);
				}
				m_StagingAllocation.reset();
			}
		};

	}
}