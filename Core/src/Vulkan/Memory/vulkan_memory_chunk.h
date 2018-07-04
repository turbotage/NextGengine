#pragma once

#include "../../def.h"
#include "../../debug.h"
#include "../vulkan_device.h"

namespace ng {
	namespace vulkan {
	

		struct Block {
			VkDeviceSize offset;
			VkDeviceSize size;

			static bool equalOffset(const Block& left, const Block& right) {
				return left.offset == right.offset;
			}

			static bool equalOffsetPlusSize(const Block& left, const Block& right) {
				return (left.offset + left.size) == (right.offset + right.size);
			}

			static bool equalOffsetPlusSizeAsOffset(const Block& left, const VkDeviceSize& right) {
				return (left.offset + left.size) == right;
			}

		};

		struct VulkanAllocationCreateInfo {
			VkDeviceSize size;
			VkDeviceSize dataSize;
		};

		class VulkanAllocation {
		public:
			VkDeviceSize offset;
			VkDeviceSize size;
			VkDeviceSize dataSize;

			static bool offsetSmallerThan(const VkDeviceSize& offset, const VulkanAllocation& block) {
				return offset < block.offset;
			}

			static bool sizeSmallerThan(const VkDeviceSize& size, const VulkanAllocation& block) {
				return size < block.size;
			}

		};

		class VulkanMemoryChunk
		{
		public:
			std::atomic<bool> hasFailedToFindMatch; //will be reset after defragmentation
			std::atomic<VkDeviceSize> lastFailedAllocateSize; //will be reset after defragmentation

			VkDeviceMemory memory;
			VkDeviceSize size;

			VkDeviceSize totalFreeSpace;

			std::list<Block> freeBlocks; // not sorted at all
			std::list<VulkanAllocation> allocations; //sorted by offset

													 /*    */
			VulkanMemoryChunk(VkDeviceSize size) {
				size = size;
			}

			VkDeviceSize getTotalFreeSpace() { return totalFreeSpace; }

			uint64 getNumFreeSpaces() { return freeBlocks.size(); }

			/* returns iterator to freeBlocks.end() if no element found */
			std::list<Block>::iterator getClosestMatch(VkDeviceSize size);

			std::list<VulkanAllocation>::iterator getClosestAllocationMatch(VkDeviceSize size);

			/* createInfo.size must be correctly aligned */
			std::shared_ptr<VulkanAllocation> allocate(VulkanAllocationCreateInfo createInfo);

			/*  Will only create the new free block, won't erase the free block corrensponding to the freeBlock argument.
			Deletion of old freeSpace should be done after this function returns, will however change the var satoring the totalFreeSpace*/
			std::shared_ptr<VulkanAllocation> allocate(Block freeBlock, VulkanAllocationCreateInfo createInfo);

			void free(const VulkanAllocation& alloc);

			void defragment(VulkanDevice* vulkanDevice);

		};

		class VulkanBufferChunk : public VulkanMemoryChunk {
		public:

			VkBuffer buffer;

			VkResult create(VulkanDevice* vulkanDevice,
				VkMemoryPropertyFlags flags,
				VkBufferUsageFlags usage)
			{

				VkBufferCreateInfo bufferInfo = {};
				bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				bufferInfo.size = size;
				bufferInfo.usage = usage;
				bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

				VkResult result = vkCreateBuffer(vulkanDevice->logicalDevice, &bufferInfo, nullptr, &buffer);
				if (result != VK_SUCCESS) {
					LOGD("failed to create chunk buffer");
					return result;
				}

				VkMemoryRequirements memRequirements;
				vkGetBufferMemoryRequirements(vulkanDevice->logicalDevice, buffer, &memRequirements);

				VkBool32 memTypeFound;
				VkMemoryAllocateInfo allocInfo = {};
				allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				allocInfo.allocationSize = memRequirements.size;
				allocInfo.memoryTypeIndex = vulkanDevice->getMemoryTypeIndex(memRequirements.memoryTypeBits, flags);
				if (allocInfo.memoryTypeIndex == -1) {
					LOGD("found no matching memory type");
					debug::exitFatal("found no matching memory type", -1);
				}

				result = vkAllocateMemory(vulkanDevice->logicalDevice, &allocInfo, nullptr, &memory);
				if (result != VK_SUCCESS) {
					LOGD("failed to create chunk memory");
					return result;
				}

				vkBindBufferMemory(vulkanDevice->logicalDevice, buffer, memory, 0);

				freeBlocks.emplace_front(0, size);
				totalFreeSpace = size;
			}

		};

		class VulkanImageChunk : public VulkanMemoryChunk {


			VkResult create(VulkanDevice* vulkanDevice,
				VkMemoryPropertyFlags flags,
				VkImageUsageFlags usage) 
			{
				
			}
		};

	}
}