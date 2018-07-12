#pragma once

#include "vulkan_memory_chunk.h"

namespace ng {
	namespace vulkan {

		struct VulkanBufferAllocationCreateInfo {
			VkDeviceSize size;
			VkDeviceSize dataSize;
		};

		class VulkanBufferAllocation {
			friend class VulkanBuffer;

			/* Ptr to the Staging VulkanAllocation corresponding to a device allocation,
			OBS!! Only used for device-local vulkan-allocations */
			VulkanBufferAllocation* m_StagingPtr = nullptr;
		public:
			VkDeviceSize offset;
			VkDeviceSize size;
			VkDeviceSize dataSize;

			static bool offsetSmallerThan(const VkDeviceSize& offset, const VulkanBufferAllocation& block) {
				return offset < block.offset;
			}

			static bool sizeSmallerThan(const VkDeviceSize& size, const VulkanBufferAllocation& block) {
				return size < block.size;
			}

		};

		class VulkanBufferChunk
		{
		public:
			std::atomic<bool> hasFailedToFindMatch; //will be reset after defragmentation
			std::atomic<VkDeviceSize> lastFailedAllocateSize; //will be reset after defragmentation

			VkDeviceMemory memory;
			VkBuffer buffer;

			VkDeviceSize size;

			VkDeviceSize totalFreeSpace;

			std::list<Block> freeBlocks; // not sorted at all
			std::list<VulkanBufferAllocation> allocations; // sorted by offset

													 /*    */
			VulkanBufferChunk(VkDeviceSize size) { this->size = size; }

			VkResult create(VulkanDevice* vulkanDevice, VkMemoryPropertyFlags flags, VkBufferUsageFlags usage);

			VkDeviceSize getTotalFreeSpace() { return totalFreeSpace; }

			uint64 getNumFreeSpaces() { return freeBlocks.size(); }

			/* returns iterator to freeBlocks.end() if no element found */
			std::list<Block>::iterator getClosestMatch(VkDeviceSize size);

			std::list<Block>::iterator getFreeBlock(VkDeviceSize offset, VkDeviceSize size);

			std::list<VulkanBufferAllocation>::iterator getClosestAllocationMatch(VkDeviceSize size);

			void changeAllocationSize(std::shared_ptr<VulkanBufferAllocation> alloc, VkDeviceSize newSize, VkDeviceSize newDataSize);

			/*  Will only create the new free block, won't erase the free block corrensponding to the freeBlock argument.
			Deletion of old freeSpace should be done after this function returns, will however change the var satoring the totalFreeSpace*/
			std::shared_ptr<VulkanBufferAllocation> allocate(Block freeBlock, VulkanBufferAllocationCreateInfo createInfo);

			void free(const VulkanBufferAllocation& alloc);

			void defragment(std::vector<VkBufferCopy>* copyRegions);

		};
	}
}
