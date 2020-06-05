#pragma once

#include "../def.h"
#include <map>
#include <memory>
#include <string>

namespace ng {

	class AbstractFreeListAllocator;

	class AbstractFreeListAllocation : public MakeConstructed<AbstractFreeListAllocation> {
	public:
		uint64 getSize();
		uint64 getOffset();

		uint64 getTotalSize();
		uint64 getPaddedOffset();

		// Make Factory
		static std::unique_ptr<AbstractFreeListAllocation> make(const std::shared_ptr<AbstractFreeListAllocator> pAllocator);
			
		~AbstractFreeListAllocation();

	private:
		// Make Factory
		AbstractFreeListAllocation(const std::shared_ptr<AbstractFreeListAllocator> pAllocator);
		AbstractFreeListAllocation(const AbstractFreeListAllocation&) = delete;
		AbstractFreeListAllocation& operator=(const AbstractFreeListAllocation&) = delete;

	private:

		friend class AbstractFreeListAllocator;
		std::weak_ptr<AbstractFreeListAllocator> m_pAllocator;

		uint64 m_PaddingOffset;
		uint64 m_TotalSize;

		uint64 m_AlignedOffset;
		uint64 m_Size;
	};

	// TODO: some form of defragmentation
	class AbstractFreeListAllocator : public MakeConstructed<AbstractFreeListAllocator> {
	public:

		static std::unique_ptr<AbstractFreeListAllocator> make(uint64 size);

		bool canAllocate(uint64 size, uint64 alignment);

		std::unique_ptr<AbstractFreeListAllocation> allocate(uint64 size, uint64 alignment);

		bool free(std::shared_ptr<AbstractFreeListAllocation> pAlloc);

		std::string getUsedBlocksString();

		std::string getFreeBlocksString();

	private:
		AbstractFreeListAllocator(uint64 size);
		AbstractFreeListAllocator(const AbstractFreeListAllocator&) = delete;
		AbstractFreeListAllocator& operator=(const AbstractFreeListAllocator&) = delete;



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