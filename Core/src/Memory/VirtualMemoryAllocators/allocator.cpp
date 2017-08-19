#include "allocator.h"

ng::memory::Allocator::~Allocator()
{
	if (m_Memory != nullptr) {
		free(m_Memory);
	}
}

void ng::memory::Allocator::init(uint64 size, uint16 allocatorIndex)
{
	m_Memory = (char*)malloc(size);
	m_MemorySize = size;

	ContiguesBlock cb;
	cb.data = m_Memory;
	cb.offset = 0;
	cb.size = 0;
	m_ContiguesBlocks.push_back(cb);
}

ng::memory::Allocation* ng::memory::Allocator::allocate(uint64 bytes)
{
	Allocation alloc;

	for (int j = 0; j < 2; ++j) {
		//TODO: find suitable place for allocation
		if (bytes < m_MemorySize - m_ContiguesBlocks.back().size) {
			//set all alloc members
			alloc.size = bytes;
			alloc.offset = m_ContiguesBlocks.back().size;
			alloc.data = (void*)(&m_Memory + alloc.offset);
			alloc.contiguesBlockIndex = m_ContiguesBlocks.size() - 1;
			alloc.allocationIndex = m_Allocations.size();

			m_ContiguesBlocks.back().size += alloc.size;
			m_Allocations.push_back(alloc);

			return &alloc;
		}
		else {
			for (int i = 0; i < m_ContiguesBlocks.size(); ++i) {
				uint64 cb1 = m_ContiguesBlocks[i].offset + m_ContiguesBlocks[i].size;
				uint64 cb2 = m_ContiguesBlocks[i + 1].offset;
				if (bytes < cb2 - cb1) {
					//set all alloc members
					alloc.size = bytes;
					alloc.offset = cb1;
					alloc.data = (void*)(&m_Memory[alloc.offset]);
					alloc.contiguesBlockIndex = i;
					alloc.allocationIndex = m_Allocations.size();

					m_ContiguesBlocks[i].size += alloc.size;
					m_Allocations.push_back(alloc);

					return &alloc;
				}
			}
		}
		if (j == 1) {
			std::runtime_error("out of memory in Allocator");
		}
		defragment();
	}
}

bool ng::memory::Allocator::freeAllocation(Allocation* alloc)
{
	//if allocation is the last in ContiguesBlock
	if ((alloc->offset + alloc->size) == (m_ContiguesBlocks[alloc->contiguesBlockIndex].offset + m_ContiguesBlocks[alloc->contiguesBlockIndex].size)){
		m_ContiguesBlocks[alloc->contiguesBlockIndex].size -= alloc->size;
		if ((m_ContiguesBlocks[alloc->contiguesBlockIndex].size == 0) && m_ContiguesBlocks.size() > 1) {
			m_ContiguesBlocks.erase(m_ContiguesBlocks.begin() + alloc->contiguesBlockIndex);
		}

	}
	//if allocation is the first in ContiguesBlock
	else if (alloc->offset == m_ContiguesBlocks[alloc->contiguesBlockIndex].offset) {
		m_ContiguesBlocks[alloc->contiguesBlockIndex].offset += alloc->size;
		if ((m_ContiguesBlocks[alloc->contiguesBlockIndex].size == 0) && m_ContiguesBlocks.size() > 1) {
			m_ContiguesBlocks.erase(m_ContiguesBlocks.begin() + alloc->contiguesBlockIndex);
		}
	}
	else {
		//if allocation isn't in the first or last in ContiguesBlock
		ContiguesBlock cb;
		cb.offset = alloc->offset + alloc->size;
		cb.size = m_ContiguesBlocks[alloc->contiguesBlockIndex].size - cb.offset;
		cb.data = (void*)(&m_Memory[alloc->offset]);

		m_ContiguesBlocks.insert(m_ContiguesBlocks.begin() + alloc->contiguesBlockIndex + 1, cb);
		for (auto& allocation : m_Allocations) {
			if (allocation.contiguesBlockIndex == alloc->contiguesBlockIndex) {
				if (allocation.offset > alloc->offset) {
					++allocation.contiguesBlockIndex;
				}
			}
		}
	}
	for (int i = alloc->allocationIndex; i < m_Allocations.size(); ++i) {
		--m_Allocations[i].allocationIndex;
	}
	m_Allocations.erase(m_Allocations.begin() + alloc->allocationIndex);
	return true;
}

bool ng::memory::Allocator::defragment()
{
	return false;
}

uint64 ng::memory::Allocator::getAllocatedMemory()
{
	uint64 mem = 0;
	for (const auto& cb : m_ContiguesBlocks) {
		mem += cb.size;
	}
	return mem;
}

std::vector<ng::memory::Allocation>* ng::memory::Allocator::getAllocations()
{
	return &m_Allocations;
}

std::vector<ng::memory::ContiguesBlock>* ng::memory::Allocator::getContiguesBlocks()
{
	return &m_ContiguesBlocks;
}
