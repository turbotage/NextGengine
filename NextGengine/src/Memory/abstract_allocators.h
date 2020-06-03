#pragma once

#include "../def.h"
#include <map>
#include <memory>
#include <string>

namespace ng {

	class AbstractFreeListAllocation {
	public:
		uint64 getSize() { return size; }
		uint64 getOffset() { return alignedOffset; }

	private:
		friend class AbstractFreeListAllocator;

		uint64 paddingOffset;
		uint64 totalSize;

		uint64 alignedOffset;
		uint64 size;
	};

	// TODO: some form of defragmentation
	class AbstractFreeListAllocator {
	public:

		AbstractFreeListAllocator(uint64 size);

		bool canAllocate(uint64 size, uint64 alignment);

		bool allocate(uint64 size, uint64 alignment, std::shared_ptr<AbstractFreeListAllocation> pAlloc);

		bool free(std::shared_ptr<AbstractFreeListAllocation> pAlloc);

		std::string getUsedBlocksString();

		std::string getFreeBlocksString();

	private:


	private:

		uint64 m_Size;

		//size, offset
		std::multimap<uint64, uint64> m_FreeBlocksBySize; // sorted by size
		//offset, size
		std::multimap<uint64, uint64> m_FreeBlocksByOffset; // sorted by offset

		//offset, size
		std::multimap<uint64, uint64> m_UsedBlocksByOffset; // sorted by offset


	};

}