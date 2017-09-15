#pragma once

#include "../../def.h"
#include <vector>

namespace ng {
	namespace memory {
/*
#define ALLOCATOR_BLOCK_SIZE 4096
#define ALLOCATOR_CHUNK_SIZE (ALLOCATOR_BLOCK_SIZE*8)
*/

		/*
		Probably a quite bad allocator for regular memory allocation, allot of unnecessary vars, and no allocation memory is stored in the memory itself
		,might change that later but it's not the most important now.
		It works pretty good for what it is made for, and that is for GPU mem allocation
		*/

		struct Allocation {
			uint64 offset;
			uint64 size;
		};

		struct AllocatedChunk {
			uint64 offset;
			uint64 size;
		};

		struct AllocationNode {
			AllocationNode* next;
			AllocatedChunk allocatedChunk;
		};

		class FreeListAllocator
		{
		private:
			AllocationNode endNode;
			std::vector<AllocationNode> allocationNodes;
		public:

			void init(uint64 size);

			Allocation allocate(uint64 size);

			bool free(Allocation allocation);

		};

	}
}
