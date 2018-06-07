#include "allocator.h"

void ng::memory::Allocator::init(uint64 size)
{	
	uint8 rest = size % MEMORY_ALIGNMENT;
	MemorySize = size - rest;
	FreeMemorySize = MemorySize;
	std::pair<uint64, uint64> freeSpace;
	freeSpace.second = 0;
	freeSpace.first = MemorySize;
	FreeSpaces.insert(freeSpace);
	printf("%" PRIu64 "\n", MemorySize);
}


