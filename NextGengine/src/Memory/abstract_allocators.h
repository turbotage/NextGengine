#pragma once

#include "../def.h"
#include <map>
#include <memory>
#include <string>
#include <mutex>

namespace ng {

	class AbstractFreeListAllocator;

	class AbstractFreeListAllocation : ng::AllocatorConstructed {
	public:
		uint64 getSize();
		uint64 getOffset();

		uint64 getTotalSize();
		uint64 getPaddedOffset();

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

		uint64 m_PaddingOffset;
		uint64 m_TotalSize;

		uint64 m_AlignedOffset;
		uint64 m_Size;
	};

	// TODO: some form of defragmentation
	class AbstractFreeListAllocator : public MakeConstructed {
	public:

		static std::unique_ptr<AbstractFreeListAllocator> make(uint64 size);

		bool canAllocate(uint64 size, uint64 alignment);

		std::unique_ptr<AbstractFreeListAllocation> allocate(uint64 size, uint64 alignment);

		void free(std::unique_ptr<AbstractFreeListAllocation> pAlloc);

		std::string getUsedBlocksString();

		std::string getFreeBlocksString();

	private:
		AbstractFreeListAllocator() = default;
		AbstractFreeListAllocator(uint64 size);
		AbstractFreeListAllocator(const AbstractFreeListAllocator&) = delete;
		AbstractFreeListAllocator& operator=(const AbstractFreeListAllocator&) = delete;

		void free(ng::raw_ptr<AbstractFreeListAllocation> pAlloc);

	private:
		friend class AbstractFreeListAllocation;

		std::mutex m_Mutex;

		uint64 m_Size;
		uint64 m_UsedSize;

		//size, offset
		std::multimap<uint64, uint64> m_FreeBlocksBySize; // sorted by size
		//offset, size
		std::multimap<uint64, uint64> m_FreeBlocksByOffset; // sorted by offset

		//offset, size
		std::multimap<uint64, uint64> m_UsedBlocksByOffset; // sorted by offset


	};

}