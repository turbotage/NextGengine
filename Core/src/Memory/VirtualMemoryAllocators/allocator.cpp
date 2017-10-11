#include "allocator.h"

void ng::memory::Allocator::init(uint64 size)
{	
	uint8 rest = size % MEMORY_ALIGNMENT;
	m_MemorySize = size - rest;
	m_FreeMemorySize = m_MemorySize;
	std::pair<uint64, uint64> freeSpace;
	freeSpace.second = 0;
	freeSpace.first = m_MemorySize;
	m_FreeSpaces.insert(freeSpace);
	printf("%" PRIu64 "\n", m_MemorySize);
}

auto ng::memory::Allocator::allocate(uint32 size)
{
	uint32 allocSize = size + MEMORY_ALIGNMENT - (size % MEMORY_ALIGNMENT); //fix size to rigth mem-alignment

	auto it = m_FreeSpaces.lower_bound(allocSize); //find the freeSpace with size nearest to wanted allocationSize
	if (it == m_FreeSpaces.end()) {
		std::runtime_error("failed to allocate\n");
	}
	Allocation alloc; //allocation to return
	alloc.offset = it->second;
	alloc.size = allocSize;

	printf("Alloc offset: %" PRIu64 " , Alloc size : %" PRIu32 "\n", alloc.offset, alloc.size); //show allocation parameters
	auto ret = m_Allocations.insert(std::pair<uint64, uint32>(alloc.offset, alloc.size)); // insert allocation in bin tree

	std::pair<uint64, uint64> freeSpace; //the new freespace
	freeSpace.first = it->first - alloc.size;
	freeSpace.second = it->second + alloc.size;

	m_FreeSpaces.erase(it); //remove the old freespace
	m_FreeSpaces.insert(freeSpace); //insert the new freespace
	m_FreeMemorySize -= allocSize;
	return ret;
}

bool ng::memory::Allocator::free(Allocation allocation)
{
	auto it = m_Allocations.find(allocation.offset);
	if (it == m_Allocations.end()) {
		std::runtime_error("failed to allocate\n");
	}
	m_Allocations.erase(it);

	auto it2 = m_FreeSpaces.find(allocation.offset + allocation.size);
	if (it2 == m_FreeSpaces.end()) {
		std::pair<uint64, uint64> freeSpace;
		freeSpace.second = allocation.offset;
		freeSpace.first = allocation.size;
		printf("New FreeSpace offset: %" PRIu64 " , New FreeSpace size : %" PRIu32 "\n", freeSpace.second, freeSpace.first); //show allocation parameters
		m_FreeSpaces.insert(freeSpace);
		return false;
	}
	std::pair<uint64, uint64> freeSpace;
	freeSpace.second = allocation.offset;
	freeSpace.first = allocation.size + it->first;
	printf("New FreeSpace offset: %" PRIu64 " , New FreeSpace size : %" PRIu32 "\n", freeSpace.second, freeSpace.first); //show allocation parameters
	m_FreeSpaces.erase(it2);
	m_FreeSpaces.insert(freeSpace);
	return true;
}

uint64 ng::memory::Allocator::getPossibleDefragSize()
{
	uint64 possDefragSize = 0;
	for (auto& freeSapce : m_FreeSpaces) {
		possDefragSize = freeSapce.first;
	}
}

uint32 ng::memory::Allocator::getDefragSpaceNum()
{
	return m_FreeSpaces.size() - 1;
}

ng::memory::DefragType ng::memory::Allocator::defragment()
{
	Allocation alloc;
	alloc.offset = 0;
	alloc.size = 0;
	if (getDefragSpaceNum() ) {
		DefragType returner;
		returner.offset = 0;
		return returner;
	}
	

}


