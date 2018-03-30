#pragma once

#include "../def.h"

#include "vulkan_buffer.h"
#include "vulkan_memory_allocator.h"

namespace ng {
	namespace graphics {
		class VulkanDevice;
	}
}

namespace ng {
	namespace memory {
		namespace vma {

			struct VulkanBufferRegionAllocatorCreateInfo {
				graphics::VulkanDevice* vulkanDevice;
				VkBuffer stagingBuffer;
				VkDeviceMemory stagingBufferMemory;
				VkDeviceSize memorySize;
				VkMemoryAlignment memoryAlignment;
			};

			class VulkanBufferRegionAllocator {
			private:

				friend VulkanMemoryAllocator;

				std::mutex m_MutexLock;

				//set on init

				graphics::VulkanDevice* m_VulkanDevice;

				VkBuffer m_StagingBuffer;

				VkDeviceMemory m_StagingBufferMemory;

				VkDeviceSize m_MemorySize;
				
				VkMemoryAlignment m_MemoryAlignment;
				
				VkDeviceSize m_FreeMemorySize;


				//end of set on init

				/**  holds all allocations done in this VkDeviceMemory instance, //offset, size **/
				//std::map<VkDeviceSize, VkDeviceSize> m_Allocations;

				struct Buffers {
				public:

					struct BufferAllocation {
						VulkanBuffer buffer;
						std::vector<VulkanBuffer*> copiedBuffers;

						BufferAllocation(VulkanBufferCreateInfo createInfo) 
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
						m_Allocations.insert(std::pair<VkDeviceSize, BufferAllocation>(buffer->m_Offset, alloc));
					}
					*/

					auto emplace(VulkanBufferCreateInfo createInfo) {
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

				} m_Buffers;

				struct FreeSpaces {
				private:
					/**  holds all free spaces left in this VkDeviceMemory instance //size, offset **/
					std::unordered_multimap<VkDeviceSize, VkDeviceSize> sortBySize;
					/**  holds all free spaces left in this VkDeviceMemory instance //offset, size **/
					std::unordered_map<VkDeviceSize, VkDeviceSize> sortByOffset;
				public:

					void insert(VkDeviceSize offset, VkDeviceSize size) {
						sortBySize.insert(std::pair<VkDeviceSize, VkDeviceSize>(size, offset));
						sortByOffset.insert(std::pair<VkDeviceSize, VkDeviceSize>(offset, size));
					}

					void erase(VkDeviceSize offset, VkDeviceSize size) {
						sortByOffset.erase(sortByOffset.find(offset));
						auto ret = sortBySize.equal_range(size);
						for (auto it = ret.first; it != ret.second; ++it) {
							if (it->second == offset) {
								sortBySize.erase(it);
								return;
							}
						}
					}

					void clear() {
						sortByOffset.clear();
						sortBySize.clear();
					}

					uint32 size() {
						return sortBySize.size();
					}

					/**  offset, size  **/
					std::pair<VkDeviceSize, VkDeviceSize> findSpaceWithOffset(VkDeviceSize offset) {
						return *(sortByOffset.find(offset));
					}

					/**  offset, size **/
					std::pair<VkDeviceSize, VkDeviceSize> findSuitableFreeSpace(VkDeviceSize size) {
						auto it = sortBySize.lower_bound(size);
						if (it == sortBySize.end()) {
							return std::pair<VkDeviceSize, VkDeviceSize>(0, 0);
						}
						return *it;
					}

				} m_FreeSpaces;

			public:

				VkDeviceMemory bufferMemory;
				VkBuffer buffer;

				VulkanBufferRegionAllocator(VulkanBufferRegionAllocatorCreateInfo createInfo);

				uint32 increaseBufferCopies(VulkanBuffer* buffer);
				
				/**  offset, size **/
				std::pair<VkDeviceSize, VkDeviceSize> findSuitableFreeSpace(VkDeviceSize size);

				uint32 getFreeSpaceCount();

				bool isInBufferRegion(VulkanBuffer* buffer);

				/**  d  **/
				VulkanBuffer createBuffer(VkDeviceSize size);

				VulkanBuffer createBuffer(VkDeviceSize freeSpaceOffset, VkDeviceSize freeSpaceSize, VkDeviceSize size);

				/**  f  **/
				void freeBuffer(VulkanBuffer* buffer);

				/**  d  **/
				void defragment(VkCommandBuffer defragCommandBuffer);

			};
		}
	}
}
