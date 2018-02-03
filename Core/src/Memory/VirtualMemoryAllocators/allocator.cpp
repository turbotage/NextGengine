#include "allocator.h"
#include "..\vulkan_memory_allocator.h"

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


