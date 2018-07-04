#pragma once

#include "vulkan_buffer_allocator.h"
#include "vulkan_memory_chunk.h"

namespace ng {
	namespace vulkan {

		struct VulkanBufferCreateInfo {
			VkDeviceSize size;
			void* data;
			VkDeviceSize dataSize;
		};

		class VulkanBuffer {
		private:
			friend class VulkanBufferAllocator;
			VulkanBufferAllocator* m_BufferAllocator;

			//should always exist
			VulkanBufferChunk* m_StagingChunk = nullptr;
			std::shared_ptr<VulkanAllocation> m_StagingAllocation = nullptr;

			VulkanBufferChunk* m_DeviceChunk = nullptr;
			std::shared_ptr<VulkanAllocation> m_DeviceAllocation = nullptr;

		public:


			/* will return false if this is host local memory only */
			bool moveToDevice() {

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