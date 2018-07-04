#pragma once

#include "../../def.h"

#include "vulkan_buffer.h"
#include "vulkan_memory_allocator.h"

namespace ng {
	namespace vulkan {
		class VulkanDevice;
	}
}

namespace ng {
	namespace vulkan {

		struct VulkanBufferRegionAllocatorCreateInfo {
			VulkanDevice* vulkanDevice;
			VkDeviceMemory vkBufferMemory;
			VkBuffer vkBuffer;
			VkDeviceSize memorySize;
			VkMemoryAlignment memoryAlignment;
		};

		class VulkanBufferRegionAllocator {
		private:
			friend VulkanMemoryAllocator;

			VulkanDevice* m_VulkanDevice; //read only

			VkDeviceMemory m_VkBufferMemory; //read only
			
			VkBuffer m_VkBuffer; //read only
			
			VkDeviceSize m_MemorySize; //read only
			
			VkMemoryAlignment m_MemoryAlignment; //read only
			
			VkDeviceSize m_FreeMemorySize;

			//end of set on init

			/**  holds all allocations done in this VkDeviceMemory instance, //offset, size **/
			//std::map<VkDeviceSize, VkDeviceSize> Allocations;

			class Buffers {
				public:

					struct BufferAllocation {
						VulkanBuffer buffer;
						std::vector<VulkanBuffer*> copiedBuffers;

						BufferAllocation(VulkanBuffer::VulkanBufferInternalCreateInfo createInfo) 
							: buffer(createInfo)
						{
							
						}
					};

				private:

					std::map<VkDeviceSize, BufferAllocation> m_Allocations;

				public:

					/*
					void insert(VulkanBuffer buffer) {
						BufferAllocation alloc;
						alloc.buffer = buffer;
						alloc.bufferCount = 1;
						Allocations.insert(std::pair<VkDeviceSize, BufferAllocation>(buffer->Offset, alloc));
					}
					*/

					auto emplace(VulkanBuffer::VulkanBufferInternalCreateInfo createInfo) {
						return m_Allocations.emplace(createInfo.offset, createInfo);
					}

					void erase(VkDeviceSize offset) {
						m_Allocations.erase(m_Allocations.find(offset));
					}

					void erase(VulkanBuffer* buffer) {
						m_Allocations.erase(m_Allocations.find(buffer->m_Offset));
					}

					void erase(std::map<VkDeviceSize, BufferAllocation>::iterator it) {
						m_Allocations.erase(it);
					}

					void clear() {
						m_Allocations.clear();
					}

					size_t size() {
						return m_Allocations.size();
					}

					auto find(BufferAllocation* bufferAlloc) {
						return m_Allocations.find(bufferAlloc->buffer.m_Offset);
					}

					auto find(VulkanBuffer* buffer) {
						return m_Allocations.find(buffer->m_Offset);
					}

					auto find(VkDeviceSize offset) {
						return m_Allocations.find(offset);
					}

					auto begin() {
						return m_Allocations.begin();
					}

					auto end() {
						return m_Allocations.end();
					}

					void setBufferCopies(VulkanBuffer* buffer, std::vector<VulkanBuffer*>* copies) {
						auto it = find(buffer);
						it->second.copiedBuffers = *copies;
					}

					uint32 increaseBufferCopies(VulkanBuffer* buffer) {
						auto it = find(buffer);
						it->second.copiedBuffers.push_back(buffer);
						return it->second.copiedBuffers.size();
					}

				};

			Buffers m_Buffers;

			struct FreeSpaces {
				private:
					/**  holds all free spaces left in this VkDeviceMemory instance //size, offset **/
					std::unordered_multimap<VkDeviceSize, VkDeviceSize> m_SortBySize;
					/**  holds all free spaces left in this VkDeviceMemory instance //offset, size **/
					std::unordered_map<VkDeviceSize, VkDeviceSize> m_SortByOffset;
				public:

					void insert(VkDeviceSize offset, VkDeviceSize size) {
						m_SortBySize.insert(std::pair<VkDeviceSize, VkDeviceSize>(size, offset));
						m_SortByOffset.insert(std::pair<VkDeviceSize, VkDeviceSize>(offset, size));
					}

					void erase(VkDeviceSize offset, VkDeviceSize size) {
						m_SortByOffset.erase(m_SortByOffset.find(offset));
						auto ret = m_SortBySize.equal_range(size);
						for (auto it = ret.first; it != ret.second; ++it) {
							if (it->second == offset) {
								m_SortBySize.erase(it);
								return;
							}
						}
					}

					void clear() {
						m_SortByOffset.clear();
						m_SortBySize.clear();
					}

					uint32 size() {
						return m_SortBySize.size();
					}

					/**  offset, size  **/
					std::pair<VkDeviceSize, VkDeviceSize> findSpaceWithOffset(VkDeviceSize offset) {
						return *(m_SortByOffset.find(offset));
					}

					/**  offset, size **/
					std::pair<VkDeviceSize, VkDeviceSize> findSuitableFreeSpace(VkDeviceSize size) {
						auto it = m_SortBySize.lower_bound(size);
						if (it == m_SortBySize.end()) {
							return std::pair<VkDeviceSize, VkDeviceSize>(0, 0);
						}
						return *it;
					}

				};

			FreeSpaces m_FreeSpaces;

			std::mutex m_Mutex;

		public:

			VulkanBufferRegionAllocator(VulkanBufferRegionAllocatorCreateInfo createInfo);

			VulkanBufferRegionAllocator(const VulkanBufferRegionAllocator &) = delete;
			VulkanBufferRegionAllocator(VulkanBufferRegionAllocator &&) = delete;

			uint32 increaseBufferCopies(VulkanBuffer* buffer);
			
			/**  offset, size **/
			std::pair<VkDeviceSize, VkDeviceSize> findSuitableFreeSpace(VkDeviceSize size);

			uint32 getFreeSpaceCount();

			bool isInBufferRegion(VulkanBuffer* buffer);
			
			/**  d  **/
			VkResult createBuffer(VulkanBuffer* vulkanBuffer, VulkanBufferCreateInfo createInfo);

			VkResult createBuffer(VulkanBuffer* vulkanBuffer, VkDeviceSize freeSpaceOffset, VkDeviceSize freeSpaceSize, VulkanBufferCreateInfo createInfo);

			/**  w  **/
			VkResult write(VulkanBuffer* vulkanBuffer, void* newData, VkDeviceSize newDataSize);

			/**  f  **/
			void freeBuffer(VulkanBuffer* buffer);

			/**  d  **/
			void defragment(VkCommandBuffer defragCommandBuffer, std::unique_lock<std::mutex>* lock);

		};
	}
}
