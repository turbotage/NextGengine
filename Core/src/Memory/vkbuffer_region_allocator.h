#pragma once

#include "../def.h"

#include <map>
#include <unordered_map>

namespace ng {
	namespace vma {

		class Buffer {
		private:
			friend class VkBufferRegionAllocator;

			std::map<VkDeviceSize, VkDeviceSize>::iterator m_OffsetAndSize;

			friend void VkBufferRegionAllocator::setBufferIt(Buffer*, std::map<VkDeviceSize, VkDeviceSize>::iterator);

		public:
			VkBuffer * buffer;

			VkDeviceSize getOffset() {
				return m_OffsetAndSize->first;
			}

			VkDeviceSize getSize() {
				return m_OffsetAndSize->second;
			}
		};

		class VkBufferRegionAllocator {
		private:
			void setBufferIt(Buffer* buffer, std::map<VkDeviceSize, VkDeviceSize>::iterator it) {
				buffer->m_OffsetAndSize = it;
			}

			VkDeviceSize m_MemorySize;
			VkDeviceSize m_FreeMemorySize;
			VkDeviceMemory m_DeviceMemory;

			VkBuffer m_Buffer;

			/**  holds all allocations done in this VkDeviceMemory instance, //offset, size **/
			std::map<VkDeviceSize, VkDeviceSize> m_Allocations;

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
						std::runtime_error("failed to allocate\n");
					}
					return *it;
				}

			} m_FreeSpaces;

		public:

			/**  d  **/
			Buffer createBuffer(VkDeviceSize size);

			/**  f  **/
			void freeBuffer(Buffer buffer);

			/**  d  **/
			void defragment();

		};
	}
}
