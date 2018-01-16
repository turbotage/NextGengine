#pragma once

#include "../../def.h"
#include <vector>
#include <mutex>
#include <map>

namespace ng {

	namespace vma {

		struct Buffer {
			VkBuffer* buffer;
			VkDeviceSize offset;
			VkDeviceSize size;
		};

		class VulkanMemoryAllocator {
		private:

			VkDevice* m_Device;
			VkPhysicalDevice* m_PhysicalDevice;

			VkBuffer m_StagingBuffer;
			VkDeviceMemory m_StagingBufferMemory;

			void createVkBufferAndMemory(
				VkBuffer* buffer, 
				VkDeviceMemory* memory, 
				VkDeviceSize size, 
				VkBufferUsageFlags usage, 
				VkMemoryPropertyFlags properties, 
				VkSharingMode sharingMode);

			class VkMemAllocator {

				VkDeviceSize m_MemorySize;
				VkDeviceSize m_FreeMemorySize;
				VkDeviceMemory m_DeviceMemory;

				VkBuffer m_Buffer;

				/**  holds all allocations done in this VkDeviceMemory instance, //offset, size **/
				std::map<VkDeviceSize, VkDeviceSize> m_Allocations;

				struct FreeSpaces {
				private:
					/**  holds all free spaces left in this VkDeviceMemory instance //size, offset **/
					std::multimap<VkDeviceSize, VkDeviceSize> sortBySize;
					/**  holds all free spaces left in this VkDeviceMemory instance //offset, size **/
					std::multimap<VkDeviceSize, VkDeviceSize> sortByOffset;
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
							}
						}
					}

					VkDeviceSize findSpaceSizeWithOffset(VkDeviceSize offset) {
						return sortByOffset.find(offset)->second;
					}

					/**  offset, size **/
					std::pair<VkDeviceSize, VkDeviceSize> findSuitableFreeSpace(VkDeviceSize size) {
						auto it = sortBySize.lower_bound(size);
						if (it == sortBySize.end()) {
							std::runtime_error("failed to allocate\n");
						}
						return std::pair<VkDeviceSize, VkDeviceSize>(it->second, it->first);
					}
				} m_FreeSpaces;

				/**  d  **/
				Buffer createBuffer(VkDeviceSize size);

				/**  f  **/
				void freeBuffer(Buffer buffer);

				/**  d  **/
				void defragment();

			};
		protected:

		public:

			Buffer createBuffer(VkDeviceSize size, VkMemoryPropertyFlags memRequirements);

			void freeBuffer(Buffer buffer);

		};
	}

	namespace memory {
/*
#define ALLOCATOR_BLOCK_SIZE 4096
#define ALLOCATOR_CHUNK_SIZE (ALLOCATOR_BLOCK_SIZE*8)
*/

#define MEMORY_ALIGNMENT 16

		/*
		The allocator used for allocations on the gpu
		*/

		struct Allocation {
			uint64 offset;
			uint32 size;
		};

		struct DefragType {
			uint64 offset;
			std::vector<Allocation> affectedAllocations;
			Allocation regionToMove;
		};

		class Allocator
		{
		protected:
			std::mutex m_AllocatorMutex;
			std::map<uint64, uint32> m_Allocations; //offset, size
			std::map<uint64, uint64> m_FreeSpaces; //size, offset
			uint64 m_MemorySize;
			uint64 m_FreeMemorySize;
		public:
			void init(uint64 size);

			auto allocate(uint32 size);

			bool free(Allocation allocation);
			
			uint64 getPossibleDefragSize();

			uint32 getDefragSpaceNum();

			ng::memory::DefragType defragment();

		};

	}
}
