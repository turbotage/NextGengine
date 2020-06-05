#pragma once

#include "vulkandef.h"
#include "vulkan_storage.h"
#include "vulkan_context.h"

#include "../Memory/abstract_allocators.h"

namespace ngv {

	class VulkanMemoryPage;

	class VulkanMemoryAllocation : public ng::MakeConstructed<VulkanMemoryAllocation> {
	public:

		// Make Factory
		static std::unique_ptr<VulkanMemoryAllocation> make(std::shared_ptr<VulkanMemoryPage> pMemPage);

		~VulkanMemoryAllocation() = default;

		vk::DeviceSize getSize();
		vk::DeviceSize getOffset();

	private:
		VulkanMemoryAllocation(std::shared_ptr<VulkanMemoryPage> pMemPage);
		VulkanMemoryAllocation(const VulkanMemoryAllocation&) = delete;
		VulkanMemoryAllocation& operator=(const VulkanMemoryAllocation&) = delete;

	private:
		friend class VulkanMemoryPage;

		std::unique_ptr<ng::AbstractFreeListAllocation> m_pAllocation;

		std::weak_ptr<VulkanMemoryPage> m_pMemoryPage;

	};

	class VulkanMemoryPage : public ng::MakeConstructed<VulkanMemoryPage> {
	public:

		// Make Factory
		static std::unique_ptr<VulkanMemoryPage> make(std::shared_ptr<vk::UniqueDeviceMemory> pMemory, vk::DeviceSize size, uint32 memoryTypeIndex);
		
		~VulkanMemoryPage() = default;

		bool canAllocate(vk::DeviceSize size, vk::DeviceSize alignment);

		std::unique_ptr<VulkanMemoryAllocation> allocate(vk::DeviceSize size, vk::DeviceSize alignment);

		bool free(std::shared_ptr<VulkanMemoryAllocation> pMemAlloc);

	private:
		VulkanMemoryPage(std::shared_ptr<vk::UniqueDeviceMemory> pMemory, vk::DeviceSize size, uint32 memoryTypeIndex);
		VulkanMemoryPage(const VulkanMemoryPage&) = delete;
		VulkanMemoryPage& operator=(const VulkanMemoryPage&) = delete;

	private:
		friend class VulkanAllocator;

		std::unique_ptr<ng::AbstractFreeListAllocator> m_pAllocator;

		std::shared_ptr<vk::UniqueDeviceMemory> m_pMemory;
		vk::DeviceSize m_Size;

		uint32 m_MemoryTypeIndex;

	};

	struct VulkanMemoryStrategy {
		vk::DeviceSize recommendedPageSize = 256 * 1000 * 1000; // 256 mb
		vk::DeviceSize maxMemoryUsage = 3 * 4 * 256 * 1000 * 1000;
	};

	class VulkanAllocator {
	public:

		VulkanAllocator(std::raw_ptr<VulkanContext> context, const VulkanMemoryStrategy& memStrategy);

		// BUFFER
		std::unique_ptr<VulkanBuffer> createBuffer(VulkanBufferCreateInfo createInfo, bool giveAllocation = true);
		bool giveBufferAllocation(std::raw_ptr<VulkanBuffer> pBuffer);
		bool looseBufferAllocation(std::raw_ptr<VulkanBuffer> pBuffer);

		// IMAGE
		std::unique_ptr<VulkanImage> createImage(VulkanImageCreateInfo createInfo, bool giveAllocation = true);
		bool giveImageAllocation(std::raw_ptr<VulkanImage> image);

	private:


	private:

		std::raw_ptr<VulkanContext> m_Context;

		VulkanMemoryStrategy m_MemoryStrategy;
		vk::DeviceSize m_UsedMemory = 0;

		struct BufferRequirements {
			vk::BufferCreateInfo createInfo;
			vk::MemoryPropertyFlags memoryPropertyFlags;
			vk::MemoryRequirements memRequirements;
		};
		std::vector<BufferRequirements> m_BufferRequirements;

		struct ImageRequirements {
			vk::ImageCreateInfo createInfo;
			vk::MemoryPropertyFlags memoryPropertyFlags;
			vk::MemoryRequirements memRequirements;
		};
		std::vector<ImageRequirements> m_ImageRequirements;


		std::list<std::shared_ptr<VulkanMemoryPage>> m_MemoryPages;

		


	};

}
