#pragma once

#include "vulkan_memory_chunk.h"
#include "vulkan_buffer_allocator.h"

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
			std::shared_ptr<VulkanBufferAllocation> m_StagingAllocation = nullptr;

			VulkanBufferChunk* m_DeviceChunk = nullptr;
			std::shared_ptr<VulkanBufferAllocation> m_DeviceAllocation = nullptr;

		public:

			bool inDeviceMemory() {
				return m_DeviceAllocation->m_StagingPtr == &(*m_StagingAllocation);
			}

			bool writeToDevice() {
				auto commandBuffer = m_BufferAllocator->m_VulkanDevice->createCommandBuffer(
					VK_COMMAND_BUFFER_LEVEL_PRIMARY,
					m_BufferAllocator->m_VulkanDevice->memoryCommandPool,
					true
				);

				VkBufferCopy copyRegion = {};
				copyRegion.srcOffset = m_StagingAllocation->offset;
				copyRegion.size = m_StagingAllocation->size;
				copyRegion.dstOffset = m_DeviceAllocation->offset;

				vkCmdCopyBuffer(commandBuffer, m_StagingChunk->buffer, m_DeviceChunk->buffer, 1, &copyRegion);

				m_BufferAllocator->m_VulkanDevice->flushCommandBuffer(
					commandBuffer,
					m_BufferAllocator->m_VulkanDevice->memoryCommandPool,
					m_BufferAllocator->m_VulkanDevice->transferQueue
				);
			}

			/* will return false if this is host local memory only */
			bool moveToDevice() {
				
				if (inDeviceMemory()) {
					return true;
				}

				//we don't need to change allocation size if this is the first time we move to device/alloc
				if (m_DeviceAllocation != nullptr) {
					m_DeviceChunk->changeAllocationSize(m_DeviceAllocation, m_StagingAllocation->size, m_StagingAllocation->dataSize);
				}
				
				m_DeviceAllocation->m_StagingPtr = &(*m_StagingAllocation);
				
				writeToDevice();
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

