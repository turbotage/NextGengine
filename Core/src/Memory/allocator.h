#pragma once

#include "../def.h"
#include <vector>
#include <mutex>
#include <map>
#include <unordered_map>

#define MEMORY_ALIGNMENT 16

namespace ng {

	namespace memory {
/*
#define ALLOCATOR_BLOCK_SIZE 4096
#define ALLOCATOR_CHUNK_SIZE (ALLOCATOR_BLOCK_SIZE*8)
*/

		/*
		The allocator used for allocations on the gpu
		*/

		struct Allocation {
			uint64 offset;
			uint32 size;
		};

		struct DefragType {
			uint64 offset;
			std::vector<Allocation> affectedAllocations;
			Allocation regionToMove;
		};

		class Allocator
		{
		protected:
			std::mutex AllocatorMutex;
			std::map<uint64, uint32> Allocations; //offset, size
			std::map<uint64, uint64> FreeSpaces; //size, offset
			uint64 MemorySize;
			uint64 FreeMemorySize;
		public:
			void init(uint64 size);

			auto allocate(uint32 size);

			bool free(Allocation allocation);
			
			uint64 getPossibleDefragSize();

			uint32 getDefragSpaceNum();

			ng::memory::DefragType defragment();

		};

	}
}
