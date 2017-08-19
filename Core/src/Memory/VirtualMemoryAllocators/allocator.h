#pragma once

#include "../../def.h"

namespace ng {
	namespace memory {

		struct ContiguesBlock {
			void* data;
			uint64 offset;
			uint64 size;
		};

		struct Allocation {
			void* data;
			uint64 offset;
			uint64 size;
			uint32 contiguesBlockIndex;
			uint32 allocationIndex;
		};

		/*
		Probably a quite bad allocator for regular memory allocation, allot of unnecessary vars, and no allocation memory is stored in the memory itself
		might, change that later but it's not the most important now.
		It works pretty good for what it is made for, and that is for GPU mem allocation
		*/
		//TODO: make allocation be 8 byte or 16 byte aligned
		class Allocator
		{
		private:
			std::vector<ContiguesBlock> m_ContiguesBlocks;
			std::vector<Allocation> m_Allocations;

			uint16 m_AllocatorIndex;
			uint64 m_MemorySize;
			char* m_Memory = nullptr;
		public:
			
			~Allocator();

			void init(uint64 size, uint16 allocatorIndex);

			Allocation* allocate(uint64 bytes);

			bool freeAllocation(Allocation* alloc);

			bool defragment();

			uint64 getAllocatedMemory();

			std::vector<Allocation>* getAllocations();

			std::vector<ContiguesBlock>* getContiguesBlocks();

		};

	}
}
