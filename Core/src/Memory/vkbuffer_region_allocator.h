#pragma once

#include "../def.h"

#include <map>
#include <unordered_map>
#include <mutex>

#include "vulkan_memory_allocator.h"

namespace ng {
	namespace vma {

		class Buffer {
		private:
			friend class VkBufferRegionAllocator;

			uint16 m_BufferCopies;
			
			VkBuffer* m_Buffer;

			void* m_Data;
			
			VkDeviceSize m_Offset;
			
			VkDeviceSize m_Size;
		public:

			Buffer();

			Buffer(VkDeviceSize offset, VkDeviceSize size, VkBuffer* buffer);

			Buffer(Buffer &buf);

		};

		

		class VkBufferRegionAllocator {
		private:

			friend VulkanMemoryAllocator;

			std::mutex m_MutexLock;

			VkMemoryAlignment m_MemoryAlignment;
			VkDeviceSize m_MemorySize;
			VkDeviceSize m_FreeMemorySize;

			/**  holds all allocations done in this VkDeviceMemory instance, //offset, size **/
			std::map<VkDeviceSize, VkDeviceSize> m_Allocations;

			struct Allocations {
			private:
				struct SizeAndBuffer {
					VkDeviceSize size;
					Buffer* buffer;

					SizeAndBuffer(VkDeviceSize size, Buffer* buffer) 
						: size(size), buffer(buffer) {};
				};
				
				std::map<VkDeviceSize, SizeAndBuffer> m_Allocations;
			
			public:

				void insert(VkDeviceSize offset, VkDeviceSize size, Buffer* buffer) {
					m_Allocations.insert(std::pair<VkDeviceSize, SizeAndBuffer>(offset, SizeAndBuffer(size, buffer)));
				}

				void erase(VkDeviceSize offset) {
					m_Allocations.erase(m_Allocations.find(offset));
				}

				void clear() {
					m_Allocations.clear();
				}

				void erase(std::map<VkDeviceSize, SizeAndBuffer>::iterator it) {
					m_Allocations.erase(it);
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

			} m_Allocations;

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

			VkBufferRegionAllocator(VkDeviceSize memorySize, VkMemoryAlignment memoryAlignment);

			/**  offset, size **/
			std::pair<VkDeviceSize, VkDeviceSize> findSuitableFreeSpace(VkDeviceSize size);

			bool isInBufferRegion(Buffer* buffer);

			/**  d  **/
			Buffer* createBuffer(VkDeviceSize size);

			Buffer* createBuffer(VkDeviceSize offset, VkDeviceSize size);

			/**  f  **/
			void freeBuffer(Buffer* buffer);

			/**  d  **/
			void defragment(VkBuffer stagingBuffer);

		};
	}
}
