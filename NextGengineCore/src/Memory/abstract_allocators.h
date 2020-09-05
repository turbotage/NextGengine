#pragma once

#include "../ng_utility.h"
#include "pch.h"

namespace ng {

	class AbstractFreeListAllocation;

	// TODO: some form of defragmentation
	MAKE_CONSTRUCTED
	class AbstractFreeListAllocator {
	public:

		static std::unique_ptr<AbstractFreeListAllocator> Make(uint64 size);

		bool CanAllocate(uint64 size, uint64 alignment);

		std::unique_ptr<AbstractFreeListAllocation> Allocate(uint64 size, uint64 alignment);

		void Free(std::unique_ptr<AbstractFreeListAllocation> pAlloc);

		uint64 GetUsedSize();

		std::string GetUsedBlocksString();

	private:
		AbstractFreeListAllocator() = default;
		AbstractFreeListAllocator(uint64 size);
		AbstractFreeListAllocator(const AbstractFreeListAllocator&) = delete;
		AbstractFreeListAllocator& operator=(const AbstractFreeListAllocator&) = delete;

		void free(ng::raw_ptr<AbstractFreeListAllocation> pAlloc);

	private:
		friend class AbstractFreeListAllocation;

		uint64 m_Size;
		uint64 m_UsedSize = 0;

		//size, offset
		std::multimap<uint64, uint64> m_FreeBlocksBySize; // sorted by size
		//offset, size
		std::multimap<uint64, uint64> m_FreeBlocksByOffset; // sorted by offset

		//offset, size
		std::multimap<uint64, uint64> m_UsedBlocksByOffset; // sorted by offset


	};







	ALLOCATOR_CONSTRUCTED
	class AbstractFreeListAllocation {
	public:
		uint64 GetSize();
		uint64 GetOffset();

		uint64 GetTotalSize();
		uint64 GetPaddedOffset();

		// Make Factory
		//static std::unique_ptr<AbstractFreeListAllocation> make(const std::raw_ptr<AbstractFreeListAllocator> pAllocator);

		~AbstractFreeListAllocation();

	private:
		// Make Factory
		AbstractFreeListAllocation() = default;
		AbstractFreeListAllocation(const ng::raw_ptr<AbstractFreeListAllocator> pAllocator);
		AbstractFreeListAllocation(const AbstractFreeListAllocation&) = delete;
		AbstractFreeListAllocation& operator=(const AbstractFreeListAllocation&) = delete;

	private:

		friend class AbstractFreeListAllocator;
		ng::raw_ptr<AbstractFreeListAllocator> m_pAllocator;

		uint64 m_PaddingOffset = 0;
		uint64 m_TotalSize = 0;

		uint64 m_AlignedOffset = 0;
		uint64 m_Size = 0;
	};

}