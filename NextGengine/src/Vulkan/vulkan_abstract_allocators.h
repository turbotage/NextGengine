#include "vulkandef.h"

#include <list>
#include <vector>

/* <======================== ALLOCATOR TYPES =======================>*/

namespace ngv {

	/* Pool Allocator */

	struct VulkanPoolAllocatorCreateInfo {
		vk::DeviceSize nBlocks;
		vk::DeviceSize blockSize; // should be aligned
	};

	struct VulkanPoolAllocation {
		vk::DeviceSize offset;
		vk::DeviceSize usedSize;
	};

	class VulkanPoolAllocator {
	public:

		void init(VulkanPoolAllocatorCreateInfo createInfo);

		bool hasFreeBlocks();

		VulkanPoolAllocation allocate();

		bool free(VulkanPoolAllocation allocation);

		uint32 getNumberOfBlocks();

		vk::DeviceSize getBlockSize();

		vk::DeviceSize getSize();

	private:
		vk::DeviceSize m_Size;
		vk::DeviceSize m_BlockSize;

		uint32 m_NumberOfBlocks;

		std::vector<uint32> m_FreeBlocks; //the int says which number in order

	};

	/* Free Allocator */

	struct VulkanFreeAllocatorCreateInfo {
		vk::DeviceSize size;
		vk::DeviceSize blockSize;
	};

	struct VulkanFreeAllocation {
		vk::DeviceSize offset;
		vk::DeviceSize usedSize;
	};

	class VulkanFreeAllocator {
	public:

		void init(VulkanFreeAllocatorCreateInfo createInfo);

		bool enoughSpaceWithoutDefrag(vk::DeviceSize requiredSize);

		bool enoughSpace(vk::DeviceSize requiredSize);

		VulkanFreeAllocation allocate(vk::DeviceSize requiredSize);

		bool free(VulkanFreeAllocation allocation);

	private:
		vk::DeviceSize m_Size;
		vk::DeviceSize m_BlockSize;

		uint32 m_NumberOfBlocks;

		struct BlockCollection {
			uint32 startBlock;
			uint32 nBlocks;
		};

		struct {
			bool operator()(const BlockCollection& a, const BlockCollection& b) {
				if (a.startBlock < b.startBlock) {
					return true;
				}
				else {
					return false;
				}
			}
		} compareBlockCollections;

		std::list<BlockCollection> m_FreeBlocks;

	};

}
