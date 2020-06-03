#pragma once
#include "vulkandef.h"

#include <memory>
#include <list>

#include "../Memory/abstract_allocators.h"

namespace ngv {

	class VulkanMemoryAllocation : ng::AbstractFreeListAllocation {
	public:

		vk::DeviceSize getSize() { return ng::AbstractFreeListAllocation::getSize(); }
		vk::DeviceSize getOffset() { return ng::AbstractFreeListAllocation::getOffset(); }

	private:

		std::shared_ptr<VulkanMemoryPage> m_MemoryPage;

	};

	class VulkanMemoryPage {
	public:

		VulkanMemoryPage(std::shared_ptr<vk::UniqueDeviceMemory>, vk::DeviceSize size);
		
	private:
		std::shared_ptr<ng::AbstractFreeListAllocator> m_Allocator;

		std::shared_ptr<vk::UniqueDeviceMemory> m_pMemory;
		vk::DeviceSize m_Size;

	};

	class VulkanAllocator {
	public:

	private:

		std::list<std::shared_ptr<VulkanMemoryPage>> memoryPages;

	};

}
