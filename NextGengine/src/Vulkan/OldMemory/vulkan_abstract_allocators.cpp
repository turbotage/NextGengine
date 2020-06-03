#include "vulkan_abstract_allocators.h"


/* Pool Allocator */

void ngv::VulkanPoolAllocator::init(VulkanPoolAllocatorCreateInfo createInfo)
{
	m_NumberOfBlocks = createInfo.nBlocks;
	m_BlockSize = createInfo.blockSize;

	m_Size = m_NumberOfBlocks * m_BlockSize;

	m_FreeBlocks.reserve(m_NumberOfBlocks);
	m_FreeBlocks.resize(m_NumberOfBlocks);

	for (uint32 i = 0; i < m_NumberOfBlocks; ++i) {
		m_FreeBlocks[i] = i;
	}
}

bool ngv::VulkanPoolAllocator::hasFreeBlocks()
{
	return !m_FreeBlocks.empty();
}

ngv::VulkanPoolAllocation ngv::VulkanPoolAllocator::allocate()
{
	ngv::VulkanPoolAllocation ret;

	ret.offset = m_FreeBlocks.back() * m_BlockSize;
	ret.usedSize = m_BlockSize;

	m_FreeBlocks.pop_back();

	return ret;
}

bool ngv::VulkanPoolAllocator::free(VulkanPoolAllocation allocation)
{
	uint32 toFree = allocation.offset / m_BlockSize;
	m_FreeBlocks.push_back(toFree);
	return VK_SUCCESS;
}

uint32 ngv::VulkanPoolAllocator::getNumberOfBlocks()
{
	return m_NumberOfBlocks;
}

vk::DeviceSize ngv::VulkanPoolAllocator::getBlockSize()
{
	return m_BlockSize;
}

vk::DeviceSize ngv::VulkanPoolAllocator::getSize()
{
	return m_Size;
}

/* Free Allocator */

void ngv::VulkanFreeAllocator::init(VulkanFreeAllocatorCreateInfo createInfo)
{
	m_NumberOfBlocks = createInfo.size / createInfo.blockSize; //size must always be a multiple of blockSize

	BlockCollection newBlockCol;
	newBlockCol.startBlock = 0;
	newBlockCol.nBlocks = m_NumberOfBlocks;

	m_FreeBlocks.push_back(newBlockCol);
}

bool ngv::VulkanFreeAllocator::enoughSpaceWithoutDefrag(vk::DeviceSize requiredSize)
{
	for (auto it = m_FreeBlocks.begin(); it != m_FreeBlocks.end(); it++) {
		if (it->nBlocks * m_BlockSize >= requiredSize) {
			return true;
		}
	}
	return false;
}

bool ngv::VulkanFreeAllocator::enoughSpace(vk::DeviceSize requiredSize)
{
	VkDeviceSize totalSize = 0;
	for (auto it = m_FreeBlocks.begin(); it != m_FreeBlocks.end(); it++) {
		totalSize += it->nBlocks * m_BlockSize;
	}
	if (totalSize >= requiredSize) {
		return true;
	}
	return false;
}

/*  should never be called without knowing it can allocate first (check with enoughSpaceWithoutDefrag first) */
ngv::VulkanFreeAllocation ngv::VulkanFreeAllocator::allocate(vk::DeviceSize requiredSize)
{
	std::list<BlockCollection>::iterator smallestCollection = m_FreeBlocks.end();

	uint32 requiredNumOfBlocks = (uint32)ceil(requiredSize / m_BlockSize);

	//first find a block that is big enough
	for (auto it = m_FreeBlocks.begin(); it != m_FreeBlocks.end(); it++) {
		if (it->nBlocks >= requiredNumOfBlocks) {
			smallestCollection = it;
			break;
		}
	}

	//find more suitable match, (smallest match)
	for (auto it = ++smallestCollection; it != m_FreeBlocks.end(); it++) {
		if (it->nBlocks >= requiredNumOfBlocks) {
			if (it->nBlocks < smallestCollection->nBlocks) {
				smallestCollection = it;
			}
		}
	}

	ngv::VulkanFreeAllocation ret;
	ret.offset = smallestCollection->startBlock * m_BlockSize;
	ret.usedSize = requiredNumOfBlocks * m_BlockSize;

	smallestCollection->startBlock += +requiredNumOfBlocks;
	smallestCollection->nBlocks -= requiredNumOfBlocks;

	return ret;
}

bool ngv::VulkanFreeAllocator::free(VulkanFreeAllocation allocation)
{
	// possible situations
	/*
	1)  BLOCK TO FREE | FREE BLOCK | ...
	2)  BLOCK TO FREE | USED BLOCK | ...
	3)  ... | FREE BLOCK | BLOCK TO FREE
	4)  ... | USED BLOCK | BLOCK TO FREE
	5)  FREE BLOCK | BLOCK TO FREE | FREE BLOCK
	6)  FREE BLOCK | BLOCK TO FREE | USED BLOCK
	7)  USED BLOCK | BLOCK TO FREE | USED BLOCK
	8)  USED BLOCK | BLOCK TO FREE | FREE BLOCK
	*/

	uint32 startBlock = allocation.offset / m_BlockSize;
	uint32 nBlocks = allocation.usedSize / m_BlockSize;

	bool prevIsFree = false;
	std::list<BlockCollection>::iterator prevBC = m_FreeBlocks.end();
	bool nextIsFree = false;
	std::list<BlockCollection>::iterator nextBC = m_FreeBlocks.end();

	//see if prev block is a free blockcollection
	for (auto it = m_FreeBlocks.begin(); it != m_FreeBlocks.end(); it++) {
		if (startBlock == (it->startBlock + it->nBlocks)) {
			prevBC = it;
			prevIsFree = true;
		}
	}

	//see if next block is a free blockcollection
	for (auto it = m_FreeBlocks.begin(); it != m_FreeBlocks.end(); it++) {
		if ((startBlock + nBlocks) == it->startBlock) {
			nextBC = it;
			nextIsFree = true;
		}
	}

	// 1) || 2)
	if (startBlock == 0) {
		if (nextIsFree) {  // 1)
			nextBC->startBlock = 0;
			nextBC->nBlocks = nextBC->nBlocks + nBlocks;
		}
		else {   // 2)
			BlockCollection newFreeBlock;
			newFreeBlock.startBlock = 0;
			newFreeBlock.nBlocks = nBlocks;
			m_FreeBlocks.push_front(newFreeBlock);
		}
		return true; //return VK_SUCCESS;
	}

	// 5)
	if (prevIsFree && nextIsFree) {
		prevBC->nBlocks = prevBC->nBlocks + nBlocks + nextBC->nBlocks;
		m_FreeBlocks.erase(nextBC);
		return true; //return VK_SUCCESS;
	}

	// 6) & 3)
	if (prevIsFree && !nextIsFree) {
		prevBC->nBlocks = prevBC->nBlocks + nBlocks;
		return true; //return VK_SUCCESS;
	}

	// 7) & 4)
	if (!prevIsFree && !nextIsFree) {
		BlockCollection newFreeBlock;
		newFreeBlock.startBlock = startBlock;
		newFreeBlock.nBlocks = nBlocks;
		m_FreeBlocks.push_front(newFreeBlock);
		return true; //return VK_SUCCESS;
	}

	// 8)
	if (!prevIsFree && nextIsFree) {
		nextBC->startBlock = startBlock;
		nextBC->nBlocks = nextBC->nBlocks + nBlocks;
		return true; //return VK_SUCCESS;
	}

	return false;
}