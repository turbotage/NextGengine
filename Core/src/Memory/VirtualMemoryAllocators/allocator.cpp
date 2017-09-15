#include "allocator.h"

void ng::memory::FreeListAllocator::init(uint64 size)
{
	endNode.allocatedChunk.offset = size;
	endNode.allocatedChunk.size = 0;
	endNode.next = nullptr;

	AllocationNode head;
	head.allocatedChunk.offset = 0;
	head.allocatedChunk.size = 0;
	head.next = &endNode;

}

ng::memory::Allocation ng::memory::FreeListAllocator::allocate(uint64 size)
{
	Allocation alloc;
	for (int i = 0; i < allocationNodes.size(); ++i) {
		if ((allocationNodes[i].next->allocatedChunk.offset) - (allocationNodes[i].allocatedChunk.offset + allocationNodes[i].allocatedChunk.size) > size) {
			alloc.offset = allocationNodes[i].allocatedChunk.offset;
			alloc.size = size;
			allocationNodes[i].allocatedChunk.size += size;
			return alloc;
		}
	}
	std::runtime_error("out of memory in allocator\n");
}

bool ng::memory::FreeListAllocator::free(Allocation allocation)
{
	for (int i = 0; i < allocationNodes.size(); ++i) {
		if ((allocationNodes[i].allocatedChunk.offset < allocation.offset) && ((allocationNodes[i].next->allocatedChunk.offset) > allocation.offset)) {
			AllocationNode allocNode;
			allocNode.allocatedChunk.offset = allocation.offset + allocation.size;
			allocNode.allocatedChunk.size = allocationNodes[i].allocatedChunk.offset + allocationNodes[i].allocatedChunk.size - allocNode.allocatedChunk.offset;
			allocationNodes[i].allocatedChunk.size = allocationNodes[i].allocatedChunk.offset - allocation.offset;
			allocationNodes.push_back(allocNode);
			return true;
		}
	}
	return false;
}
