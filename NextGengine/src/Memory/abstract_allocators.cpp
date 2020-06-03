#include "abstract_allocators.h"

ng::AbstractFreeListAllocator::AbstractFreeListAllocator(uint64 size)
{
	this->m_Size = size;
	m_FreeBlocksByOffset.emplace(0, size);
	m_FreeBlocksBySize.emplace(size, 0);
}

bool ng::AbstractFreeListAllocator::canAllocate(uint64 size, uint64 alignment)
{
	auto it = m_FreeBlocksBySize.lower_bound(size + alignment);
	if (it != m_FreeBlocksBySize.end()) {
		return true;
	}
	return false;
}

bool ng::AbstractFreeListAllocator::allocate(uint64 size, uint64 alignment, std::shared_ptr<AbstractFreeListAllocation> pAlloc)
{
	auto freeBlock = m_FreeBlocksBySize.lower_bound(size + alignment);
	if (freeBlock != m_FreeBlocksBySize.end()) {
		pAlloc->paddingOffset = freeBlock->second; // freeBlock offset
		pAlloc->alignedOffset = (freeBlock->second + alignment) - (freeBlock->second % alignment);
		pAlloc->totalSize = size + (pAlloc->alignedOffset - pAlloc->paddingOffset);
		pAlloc->size = size;

		uint64_t newFreeBlockOffset = (pAlloc->alignedOffset + pAlloc->size);
		uint64_t newFreeBlockSize = (freeBlock->first + freeBlock->second) - newFreeBlockOffset;

		// remove this freeblock from both sorted lists
		m_FreeBlocksByOffset.erase(freeBlock->second);
		m_FreeBlocksBySize.erase(freeBlock);

		// add a new freeBlock
		if (!(newFreeBlockSize < 1)) {
			m_FreeBlocksBySize.emplace(newFreeBlockSize, newFreeBlockOffset);
			m_FreeBlocksByOffset.emplace(newFreeBlockOffset, newFreeBlockSize);
		}

		m_UsedBlocksByOffset.emplace(pAlloc->paddingOffset, pAlloc->totalSize);
	}
	return false;
}

bool ng::AbstractFreeListAllocator::free(std::shared_ptr<AbstractFreeListAllocation> pAlloc)
{
	auto usedBlock = m_UsedBlocksByOffset.find(pAlloc->paddingOffset);
	if (usedBlock != m_UsedBlocksByOffset.end()) {
		// 4 cases (1: nether left block nor right block is free) (2: left block is free right block isn't) 
		//			(3: right block is free left block isn't) (4: no adjacent blocks are free)
		bool leftBlockIsUsed;
		bool rightBlockIsUsed;
		uint64_t newFreeBlockOffset;
		uint64_t newFreeBlockSize;

		//left block
		{

			if (usedBlock != m_UsedBlocksByOffset.begin()) {
				auto prevUsedBlock = std::prev(usedBlock);
				if (prevUsedBlock->second + prevUsedBlock->first == usedBlock->first) { // left block is a used block
					leftBlockIsUsed = true;
					newFreeBlockOffset = usedBlock->first;
				}
				else { // the left block is a free block we should find it
					leftBlockIsUsed = false;
					auto leftFreeBlock = std::prev(m_FreeBlocksByOffset.lower_bound(usedBlock->first)); // the left block should be the block just smaller than the one just bigger than our used block
					newFreeBlockOffset = leftFreeBlock->first; // our new free block will begin at the left block
					//now free the left block from both freeBlocks lists
					auto freeRange = m_FreeBlocksBySize.equal_range(leftFreeBlock->second);
					for (auto i = freeRange.first; i != freeRange.second; ++i) {
						if (i->second == leftFreeBlock->first) { // size sorted freeblock had same offset (correct one)
							m_UsedBlocksByOffset.erase(i);
							break;
						}
					}
					m_FreeBlocksByOffset.erase(leftFreeBlock);
				}
			}
			else { // there is no left used block
				newFreeBlockOffset = 0;
				if (usedBlock->first != 0) { // there is a free block to the left
					auto leftFreeBlock = m_FreeBlocksByOffset.find(0);
					auto freeRange = m_FreeBlocksBySize.equal_range(leftFreeBlock->second);
					for (auto i = freeRange.first; i != freeRange.second; ++i) {
						if (i->second == leftFreeBlock->first) {
							m_FreeBlocksBySize.erase(i);
							break;
						}
					}
					m_FreeBlocksByOffset.erase(leftFreeBlock);
				}
			}

		}

		//right block
		{

			if (usedBlock != std::prev(m_UsedBlocksByOffset.end())) {
				auto nextUsedBlock = std::next(usedBlock);
				if ((usedBlock->first + usedBlock->second) == nextUsedBlock->first) { // the right block is a used block
					rightBlockIsUsed = true;
					newFreeBlockSize = (usedBlock->first + usedBlock->second) - newFreeBlockOffset;
				}
				else { // the left block is a free block
					rightBlockIsUsed = false;
					auto rightFreeBlock = m_FreeBlocksByOffset.find(usedBlock->first + usedBlock->second); // this isn't the last usedBlock //should never be end
					newFreeBlockSize = (rightFreeBlock->first + rightFreeBlock->second) - newFreeBlockOffset;

					auto freeRange = m_FreeBlocksBySize.equal_range(rightFreeBlock->second);
					for (auto i = freeRange.first; i != freeRange.second; ++i) {
						if (i->second == rightFreeBlock->second) {
							m_FreeBlocksBySize.erase(i);
							break;
						}
					}
					m_FreeBlocksByOffset.erase(rightFreeBlock);
				}
			}
			else {
				newFreeBlockSize = this->m_Size - newFreeBlockOffset;
				if ((usedBlock->first + usedBlock->second) != this->m_Size) { // there is a free block to the right
					auto rightFreeBlock = m_FreeBlocksByOffset.find((usedBlock->first + usedBlock->second));
					auto freeRange = m_FreeBlocksBySize.equal_range(rightFreeBlock->second);
					for (auto i = freeRange.first; i != freeRange.second; ++i) {
						if (i->second == rightFreeBlock->first) {
							m_FreeBlocksBySize.erase(i);
							break;
						}
					}
					m_FreeBlocksByOffset.erase(rightFreeBlock);
				}
			}

		}

		//free the used block
		m_UsedBlocksByOffset.erase(usedBlock);

		//add the new block
		m_FreeBlocksBySize.emplace(newFreeBlockSize, newFreeBlockOffset);
		m_FreeBlocksByOffset.emplace(newFreeBlockOffset, newFreeBlockSize);

		// reset allocation
		pAlloc->alignedOffset = 0;
		pAlloc->paddingOffset = 0;
		pAlloc->totalSize = 0;
		pAlloc->size = 0;

		return true;
	}
	return false;
}

std::string ng::AbstractFreeListAllocator::getUsedBlocksString()
{
	std::string ret = "USED BLOCKS: ";
	for (auto& block : m_UsedBlocksByOffset) {
		ret += std::string(" (") + std::to_string(block.first) + std::string(",") + std::to_string(block.second) + std::string(") ");
	}
	return ret;
}

std::string ng::AbstractFreeListAllocator::getFreeBlocksString()
{
	std::string ret = "FREE BLOCKS: ";
	for (auto& block : m_FreeBlocksByOffset) {
		ret += std::string(" (") + std::to_string(block.first) + std::string(",") + std::to_string(block.second) + std::string(") ");
	}
	return ret;
}
