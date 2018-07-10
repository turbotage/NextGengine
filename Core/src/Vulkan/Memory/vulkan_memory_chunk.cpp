#include "vulkan_memory_chunk.h"

std::list<ng::vulkan::Block>::iterator ng::vulkan::VulkanMemoryChunk::getClosestMatch(VkDeviceSize size) {

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

std::list<ng::vulkan::Block>::iterator ng::vulkan::VulkanMemoryChunk::getFreeBlock(VkDeviceSize offset, VkDeviceSize size)
{
	for (auto it = freeBlocks.begin(); it != freeBlocks.end(); ++it) {
		if ((it->offset + it->size) == (offset + size)) {
			return it;
		}
	}
}

std::list<ng::vulkan::VulkanAllocation>::iterator ng::vulkan::VulkanMemoryChunk::getClosestAllocationMatch(VkDeviceSize size)
{
	std::list<VulkanAllocation>::iterator smallestIt = allocations.end();

	for (auto it = allocations.begin(); it != allocations.end(); ++it) {
		if (it->size > size) {
			smallestIt = it;
		}
	}

	for (auto it = allocations.begin(); it != allocations.end(); ++it ) {
		if ((it->size > size) && (it->size < smallestIt->size)) {
			smallestIt = it;
		}
	}

	if (smallestIt == allocations.end()) {
		LOGD("found no matching allocation for block swap");
	}
	return smallestIt;
}

void ng::vulkan::VulkanMemoryChunk::changeAllocationSize(std::shared_ptr<VulkanAllocation> alloc, VkDeviceSize newSize, VkDeviceSize newDataSize)
{
	auto it = getFreeBlock(alloc->offset, alloc->size);
	
	alloc->size = newSize;
	alloc->dataSize = newDataSize;

	VkDeviceSize newFreeBlockOffset = alloc->offset + alloc->size;
	VkDeviceSize newFreeBlockSize = it->offset + it->size - newFreeBlockOffset;

	freeBlocks.erase(it);
	freeBlocks.emplace_front(newFreeBlockOffset, newFreeBlockSize);
}

std::shared_ptr<ng::vulkan::VulkanAllocation> ng::vulkan::VulkanMemoryChunk::allocate(VulkanAllocationCreateInfo createInfo) {
	std::shared_ptr<VulkanAllocation> ret;

	auto freeBlockIt = getClosestMatch(createInfo.size);
	if (freeBlockIt == freeBlocks.end()) {
		/* this might happen with this method, shouldn't happen with other overloaded version of allocate since
		you have to give it the freespace to allocate from. Therefore the other allocate method should be  */
		hasFailedToFindMatch = true;
		lastFailedAllocateSize = createInfo.size;
		ret = nullptr;
	}

	//if freespace is in front or in back
	if ((freeBlockIt->offset + freeBlockIt->size == this->size) || (freeBlockIt->offset == 0)) {
		//case 1, freespace is in front
		if ((freeBlockIt->offset + freeBlockIt->size == this->size)) {
			ret = std::make_shared<VulkanAllocation>(&allocations.emplace_back(freeBlockIt->offset, createInfo.size, createInfo.dataSize));

			freeBlocks.emplace_front(ret->offset + ret->size, freeBlockIt->size - ret->size);

			totalFreeSpace = totalFreeSpace - freeBlockIt->size + ret->size;

			freeBlocks.erase(freeBlockIt);
			return ret;
		}
		//case 2, freespace is in back
		else {
			ret = std::make_shared<VulkanAllocation>(&allocations.emplace_front(freeBlockIt->offset, freeBlockIt->size, createInfo.dataSize));

			freeBlocks.emplace_front(ret->offset + ret->size, freeBlockIt->size - ret->size);

			totalFreeSpace = totalFreeSpace - freeBlockIt->size + ret->size;

			freeBlocks.erase(freeBlockIt);
			return ret;
		}
	}

	//case 3, freespace is somewhere between front and back
	for (auto it = allocations.begin(); it != allocations.end(); ++it) { //OBS now last iteration checks same thing as case 2
		if (freeBlockIt->offset == (it->offset + it->size)) {
			ret = std::make_shared<VulkanAllocation>(*allocations.emplace(it++, freeBlockIt->offset, freeBlockIt->size, createInfo.dataSize));

			freeBlocks.emplace_front(ret->offset + ret->size, freeBlockIt->size - ret->size);

			totalFreeSpace = totalFreeSpace - freeBlockIt->size + ret->size;

			freeBlocks.erase(freeBlockIt);
			return ret;
		}
	}

	LOGD("found suitable freespace but failed to find matchin allocation!");
	debug::exitFatal("found suitable freespace but failed to find matchin allocation!", -1);
	//won't happen, written it anyway
	return ret;
}

std::shared_ptr<ng::vulkan::VulkanAllocation> ng::vulkan::VulkanMemoryChunk::allocate(Block freeBlock, VulkanAllocationCreateInfo createInfo) {
	std::shared_ptr<VulkanAllocation> ret;

	//if freespace is in front or in back
	if ((freeBlock.offset + freeBlock.size == this->size) || (freeBlock.offset == 0)) {
		if ((freeBlock.offset + freeBlock.size == this->size)) {
			ret = std::make_shared<VulkanAllocation>(&allocations.emplace_back(freeBlock.offset, createInfo.size, createInfo.dataSize));

			freeBlocks.emplace_front(ret->offset + ret->size, freeBlock.size - ret->size);

			totalFreeSpace = totalFreeSpace - freeBlock.size + ret->size;

			return ret;
		}
		else {
			ret = std::make_shared<VulkanAllocation>(&allocations.emplace_front(freeBlock.offset, freeBlock.size, createInfo.dataSize));

			freeBlocks.emplace_front(ret->offset + ret->size, freeBlock.size - ret->size);

			totalFreeSpace = totalFreeSpace - freeBlock.size + ret->size;

			return ret;
		}
	}

	auto lastIt = allocations.begin();
	for (auto it = lastIt; it != allocations.end(); ++it) {
		if (freeBlock.offset == (it->offset + it->size)) {
			ret = std::make_shared<VulkanAllocation>(*allocations.emplace(it++, freeBlock.offset, freeBlock.size, createInfo.dataSize));

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

void ng::vulkan::VulkanMemoryChunk::free(const VulkanAllocation& alloc) {

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

void ng::vulkan::VulkanMemoryChunk::defragment(std::vector<VulkanCopyRegion>* copyRegions) {
	freeBlocks.erase(freeBlocks.begin(), freeBlocks.end());

	VkDeviceSize offset = 0;
	VulkanCopyRegion tempCopy;

	for (auto it = allocations.begin(); it != allocations.end(); ++it) {
		tempCopy.srcOffset = it->offset;
		tempCopy.dstOffset = offset;
		tempCopy.size = it->size;

		it->offset = offset;

		copyRegions->emplace_back(tempCopy.srcOffset, tempCopy.dstOffset, tempCopy.size);
		offset = it->offset + it->size;
	}

	freeBlocks.emplace_front(0, this->size);
}
