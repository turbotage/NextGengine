#pragma once

#include "../../def.h"
#include <vector>
#include <mutex>
#include <map>

namespace ng {
	namespace memory {
/*
#define ALLOCATOR_BLOCK_SIZE 4096
#define ALLOCATOR_CHUNK_SIZE (ALLOCATOR_BLOCK_SIZE*8)
*/

#define MEMORY_ALIGNMENT 16

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
			std::mutex m_AllocatorMutex;
			std::map<uint64, uint32> m_Allocations; //offset, size
			std::map<uint64, uint64> m_FreeSpaces; //size, offset
			uint64 m_MemorySize;
			uint64 m_FreeMemorySize;
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
