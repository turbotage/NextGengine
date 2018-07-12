#include "vulkan_buffer_chunk.h"

std::list<ng::vulkan::Block>::iterator ng::vulkan::VulkanBufferChunk::getClosestMatch(VkDeviceSize size) {

	auto smallestIt = freeBlocks.end();

	//first find first block bigger than size
	for (auto it = freeBlocks.begin(); it != freeBlocks.end(); ++it) {
		if (it->size > size) {
			smallestIt = it;
			break;
		}
	}
	//start searching for the smallest one from the first found block
	for (auto it = ++smallestIt; it != freeBlocks.end(); ++it) {
		if (it->size > size) {
			if (it->size < smallestIt->size) {
				smallestIt = it;
			}
		}
	}

	return smallestIt;
}

std::list<ng::vulkan::Block>::iterator ng::vulkan::VulkanBufferChunk::getFreeBlock(VkDeviceSize offset, VkDeviceSize size)
{
	for (auto it = freeBlocks.begin(); it != freeBlocks.end(); ++it) {
		if ((it->offset + it->size) == (offset + size)) {
			return it;
		}
	}
}

std::list<ng::vulkan::VulkanBufferAllocation>::iterator ng::vulkan::VulkanBufferChunk::getClosestAllocationMatch(VkDeviceSize size)
{
	std::list<VulkanBufferAllocation>::iterator smallestIt = allocations.end();

	for (auto it = allocations.begin(); it != allocations.end(); ++it) {
		if (it->size > size) {
			smallestIt = it;
		}
	}

	for (auto it = allocations.begin(); it != allocations.end(); ++it) {
		if ((it->size > size) && (it->size < smallestIt->size)) {
			smallestIt = it;
		}
	}

	if (smallestIt == allocations.end()) {
		LOGD("found no matching allocation for block swap");
	}
	return smallestIt;
}

void ng::vulkan::VulkanBufferChunk::changeAllocationSize(std::shared_ptr<VulkanBufferAllocation> alloc, VkDeviceSize newSize, VkDeviceSize newDataSize)
{
	auto it = getFreeBlock(alloc->offset, alloc->size);

	alloc->size = newSize;
	alloc->dataSize = newDataSize;

	VkDeviceSize newFreeBlockOffset = alloc->offset + alloc->size;
	VkDeviceSize newFreeBlockSize = it->offset + it->size - newFreeBlockOffset;

	freeBlocks.erase(it);
	freeBlocks.emplace_front(newFreeBlockOffset, newFreeBlockSize);
}

std::shared_ptr<ng::vulkan::VulkanBufferAllocation> ng::vulkan::VulkanBufferChunk::allocate(Block freeBlock, VulkanBufferAllocationCreateInfo createInfo) {
	std::shared_ptr<VulkanBufferAllocation> ret;

	//if freespace is in front or in back
	if ((freeBlock.offset + freeBlock.size == this->size) || (freeBlock.offset == 0)) {
		if ((freeBlock.offset + freeBlock.size == this->size)) {
			ret = std::make_shared<VulkanBufferAllocation>(&allocations.emplace_back(freeBlock.offset, createInfo.size, createInfo.dataSize));

			freeBlocks.emplace_front(ret->offset + ret->size, freeBlock.size - ret->size);

			totalFreeSpace = totalFreeSpace - freeBlock.size + ret->size;

			return ret;
		}
		else {
			ret = std::make_shared<VulkanBufferAllocation>(&allocations.emplace_front(freeBlock.offset, freeBlock.size, createInfo.dataSize));

			freeBlocks.emplace_front(ret->offset + ret->size, freeBlock.size - ret->size);

			totalFreeSpace = totalFreeSpace - freeBlock.size + ret->size;

			return ret;
		}
	}

	auto lastIt = allocations.begin();
	for (auto it = lastIt; it != allocations.end(); ++it) {
		if (freeBlock.offset == (it->offset + it->size)) {
			ret = std::make_shared<VulkanBufferAllocation>(*allocations.emplace(it++, freeBlock.offset, freeBlock.size, createInfo.dataSize));

			freeBlocks.emplace_front(ret->offset + ret->size, freeBlock.size - ret->size);

			totalFreeSpace = totalFreeSpace - freeBlock.size + ret->size;

			return ret;
		}
	}

	LOGD("no allocation matched given freeBlock, not correct or non existing parameter?");
	hasFailedToFindMatch = true;
	lastFailedAllocateSize = createInfo.size;
	ret = nullptr;
	return ret;
}

void ng::vulkan::VulkanBufferChunk::free(const VulkanBufferAllocation& alloc) {

	bool hasLeftSpace = true;
	bool hasRightSpace = false;

	std::list<Block>::iterator freeBlockIt;

	Block leftFreeBlock = { 0,0 };
	if (alloc.offset != 0) {
		freeBlockIt = std::find_if(freeBlocks.begin(), freeBlocks.end(), std::bind(Block::equalOffsetPlusSizeAsOffset, std::placeholders::_1, alloc.offset));
		if (freeBlockIt != freeBlocks.end()) {
			hasLeftSpace = true;
			leftFreeBlock = *freeBlockIt;
			freeBlocks.erase(freeBlockIt);
		}
	}

	Block rightFreeBlock = { 0, 0 };
	if (alloc.offset + alloc.size != this->size) {
		rightFreeBlock = { alloc.offset + alloc.size, 0 };
		freeBlockIt = std::find_if(freeBlocks.begin(), freeBlocks.end(), std::bind(Block::equalOffset, std::placeholders::_1, rightFreeBlock));
		if (freeBlockIt != freeBlocks.end()) {
			hasRightSpace = true;
			rightFreeBlock = *freeBlockIt;
			freeBlocks.erase(freeBlockIt);
		}
	}

	Block newFreeBlock;

	if (hasLeftSpace) {
		//case 1
		if (hasRightSpace) {
			newFreeBlock = { leftFreeBlock.offset, leftFreeBlock.size + alloc.size + rightFreeBlock.size };
		}
		//case 2
		else {
			newFreeBlock = { leftFreeBlock.offset, leftFreeBlock.size + alloc.size };
		}
	}
	else if (hasRightSpace) {
		//case 3
		if (hasLeftSpace) {
			//do same as case 1
			newFreeBlock = { leftFreeBlock.offset, leftFreeBlock.size + alloc.size + rightFreeBlock.size };
		}
		//case 4
		else {
			newFreeBlock = { alloc.offset, alloc.size + rightFreeBlock.size };
		}
	}
	//case 5
	else {
		newFreeBlock = { alloc.offset, alloc.size };
	}

	allocations.erase(std::find(allocations.begin(), allocations.end(), alloc));
	freeBlocks.emplace_front(newFreeBlock.offset, newFreeBlock.size);

}

void ng::vulkan::VulkanBufferChunk::defragment(std::vector<VkBufferCopy>* copyRegions) {
	freeBlocks.erase(freeBlocks.begin(), freeBlocks.end());

	VkDeviceSize offset = 0;

	for (auto it = allocations.begin(); it != allocations.end(); ++it) {
		copyRegions->emplace_back(it->offset, offset, it->size);

		it->offset = offset;

		offset = it->offset + it->size;
	}

	freeBlocks.emplace_front(0, this->size);
}

VkResult ng::vulkan::VulkanBufferChunk::create(VulkanDevice* vulkanDevice,
	VkMemoryPropertyFlags flags,
	VkBufferUsageFlags usage) {

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

	freeBlocks.emplace_front(0, this->size);
	totalFreeSpace = size;
}

