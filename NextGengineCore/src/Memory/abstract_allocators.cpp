#include "abstract_allocators.h"
#include "abstract_allocators.h"
#include "abstract_allocators.h"
#include "abstract_allocators.h"
#include "abstract_allocators.h"
#include "abstract_allocators.h"
#include "abstract_allocators.h"
#include "abstract_allocators.h"

// <======================= ABSTRACT FREE LIST ALLOCATOR =================================>
// public
std::unique_ptr<ng::AbstractFreeListAllocator> ng::AbstractFreeListAllocator::Make(uint64 size)
{
	return std::unique_ptr<AbstractFreeListAllocator>(new AbstractFreeListAllocator(size));
}

bool ng::AbstractFreeListAllocator::CanAllocate(uint64 size, uint64 alignment)
{
	auto it = m_FreeBlocksBySize.lower_bound(size + alignment);
	if (it != m_FreeBlocksBySize.end()) {
		return true;
	}
	return false;
}

std::unique_ptr<ng::AbstractFreeListAllocation> ng::AbstractFreeListAllocator::Allocate(uint64 size, uint64 alignment)
{
	auto freeBlock = m_FreeBlocksBySize.lower_bound(size + alignment);
	if (freeBlock != m_FreeBlocksBySize.end()) {
		std::unique_ptr<AbstractFreeListAllocation> pAlloc(new AbstractFreeListAllocation());
		pAlloc->m_pAllocator = this;
		pAlloc->m_PaddingOffset = freeBlock->second; // freeBlock offset
		//set aligned offset
		{
			uint64 rem = (freeBlock->second + alignment) % alignment;
			if (rem == 0) {
				pAlloc->m_AlignedOffset = freeBlock->second;
			}
			else {
				pAlloc->m_AlignedOffset = freeBlock->second + alignment - rem;
			}
		}
		pAlloc->m_TotalSize = size + (pAlloc->m_AlignedOffset - pAlloc->m_PaddingOffset);
		pAlloc->m_Size = size;

		uint64_t newFreeBlockOffset = (pAlloc->m_AlignedOffset + pAlloc->m_Size);
		uint64_t newFreeBlockSize = (freeBlock->first + freeBlock->second) - newFreeBlockOffset;

		// remove this freeblock from both sorted lists
		m_FreeBlocksByOffset.erase(freeBlock->second);
		m_FreeBlocksBySize.erase(freeBlock);

		// add a new freeBlock
		if (!(newFreeBlockSize < 1)) {
			m_FreeBlocksBySize.emplace(newFreeBlockSize, newFreeBlockOffset);
			m_FreeBlocksByOffset.emplace(newFreeBlockOffset, newFreeBlockSize);
		}

		m_UsedBlocksByOffset.emplace(pAlloc->m_PaddingOffset, pAlloc->m_TotalSize);

		m_UsedSize += pAlloc->m_TotalSize;
		return pAlloc;
	}
	return nullptr;
}

void ng::AbstractFreeListAllocator::Free(std::unique_ptr<ng::AbstractFreeListAllocation> pAlloc)
{
	free(pAlloc.get());
}

uint64 ng::AbstractFreeListAllocator::GetUsedSize()
{
	return m_UsedSize;
}

std::string ng::AbstractFreeListAllocator::GetUsedBlocksString()
{
	std::string ret = "USED BLOCKS: ";
	for (auto& block : m_UsedBlocksByOffset) {
		ret += std::string(" (") + std::to_string(block.first) + std::string(",") + std::to_string(block.second) + std::string(") ");
	}
	return ret;
}



//private
ng::AbstractFreeListAllocator::AbstractFreeListAllocator(uint64 size)
{
	this->m_Size = size;
	m_FreeBlocksByOffset.emplace(0, size);
	m_FreeBlocksBySize.emplace(size, 0);
}

void ng::AbstractFreeListAllocator::free(ng::raw_ptr<AbstractFreeListAllocation> pAlloc)
{

#ifndef NDEBUG
	if (pAlloc->m_pAllocator != this) {
		std::runtime_error("Tried to free non allocated FreeList allocation");
	}
#endif

	auto usedBlock = m_UsedBlocksByOffset.find(pAlloc->m_PaddingOffset);

#ifndef NDEBUG
	if (usedBlock != m_UsedBlocksByOffset.end()) {
		std::runtime_error("FreeList allocation to be freed not found");
	}
#endif

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


	// decrease used size
	m_UsedSize -= pAlloc->m_PaddingOffset;

	// reset allocation
	pAlloc->m_pAllocator = nullptr;
	pAlloc->m_AlignedOffset = 0;
	pAlloc->m_PaddingOffset = 0;
	pAlloc->m_TotalSize = 0;
	pAlloc->m_Size = 0;

}












// <====================== ABSTRACT FREE LIST ALLOCATION ============================>
// public
uint64 ng::AbstractFreeListAllocation::GetSize() {
	return m_Size;
}

uint64 ng::AbstractFreeListAllocation::GetOffset()
{
	return m_AlignedOffset;
}

uint64 ng::AbstractFreeListAllocation::GetTotalSize()
{
	return m_TotalSize;
}

uint64 ng::AbstractFreeListAllocation::GetPaddedOffset()
{
	return m_PaddingOffset;
}


ng::AbstractFreeListAllocation::~AbstractFreeListAllocation()
{
	m_pAllocator->free(this);
}

//private
ng::AbstractFreeListAllocation::AbstractFreeListAllocation(const ng::raw_ptr<AbstractFreeListAllocator> pAllocator)
{
	m_pAllocator = pAllocator;
}


