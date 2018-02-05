#pragma once

#include "../def.h"

#include <map>
#include <unordered_map>
#include <mutex>

#include "buffer.h"
#include "vulkan_memory_allocator.h"

namespace ng {
	namespace vma {

		struct VkBufferRegionAllocatorCreateInfo {
			VkDevice* device;
			VkBuffer* stagingBuffer;
			VkCommandPool* commandPool;
			VkQueue* queue;
			VkDeviceMemory* stagingBufferMemory;
			VkDeviceSize memorySize;
			VkMemoryAlignment memoryAlignment;
		};

		class VkBufferRegionAllocator {
		private:

			friend VulkanMemoryAllocator;

			std::mutex m_MutexLock;

			VkBufferRegionAllocatorCreateInfo m_CreateInfo;

			VkDeviceSize m_FreeMemorySize;

			/**  holds all allocations done in this VkDeviceMemory instance, //offset, size **/
			std::map<VkDeviceSize, VkDeviceSize> m_Allocations;

			struct Buffers {
			private:
				
				std::map<VkDeviceSize, Buffer*> m_Allocations;
			
			public:

				void insert(Buffer* buffer) {
					m_Allocations.insert(std::pair<VkDeviceSize, Buffer*>(buffer->m_Offset, buffer));
				}

				void erase(VkDeviceSize offset) {
					m_Allocations.erase(m_Allocations.find(offset));
				}

				void erase(Buffer* buffer) {
					m_Allocations.erase(m_Allocations.find(buffer->m_Offset));
				}

				void erase(std::map<VkDeviceSize, Buffer*>::iterator it) {
					m_Allocations.erase(it);
				}

				void clear() {
					m_Allocations.clear();
				}

				size_t size() {
					return m_Allocations.size();
				}

				auto find(Buffer* buffer) {
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

			} m_Buffers;

			struct FreeSpaces {
			private:
				/**  holds all free spaces left in this VkDeviceMemory instance //size, offset **/
				std::unordered_multimap<VkDeviceSize, VkDeviceSize> sortBySize;
				/**  holds all free spaces left in this VkDeviceMemory instance //offset, size **/
				std::unordered_multimap<VkDeviceSize, VkDeviceSize> sortByOffset;
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

			VkBufferRegionAllocator(VkBufferRegionAllocatorCreateInfo createInfo);

			/**  offset, size **/
			std::pair<VkDeviceSize, VkDeviceSize> findSuitableFreeSpace(VkDeviceSize size);

			bool isInBufferRegion(Buffer* buffer);

			/**  d  **/
			Buffer* createBuffer(VkDeviceSize size);

			Buffer* createBuffer(VkDeviceSize offset, VkDeviceSize size);

			/**  f  **/
			void freeBuffer(Buffer* buffer);

			/**  d  **/
			void defragment();

		};
	}
}
