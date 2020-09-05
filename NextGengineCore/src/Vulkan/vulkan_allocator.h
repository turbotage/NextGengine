#pragma once

#include "pch.h"

#include "vulkan_storage.h"
#include "vulkan_context.h"

#include "../Memory/abstract_allocators.h"

namespace ngv {

	class VulkanMemoryAllocation;
	class VulkanMemoryPage;



	struct VulkanMemoryStrategy {
		vk::DeviceSize recommendedPageSize = 256 * 1024 * 1024LL; // 256 mb
		vk::DeviceSize maxMemoryUsage = 3LL * 4LL * 256LL * 1024LL * 1024LL;
	};

	class VulkanAllocator {
	public:

		VulkanAllocator(VulkanDevice& device, const VulkanMemoryStrategy& memStrategy);

		// BUFFER
		void GiveBufferAllocation(VulkanBuffer& buffer);

		// IMAGE
		void GiveImageAllocation(VulkanImage& pImage);


		vk::DeviceSize GetUsedMemory();

	private:

		void mGiveBufferAllocation(VulkanBuffer& buffer);

		void mGiveImageAllocation(VulkanImage& image);

		vk::DeviceSize mGetUsedMemory();

	private:

		std::mutex m_Mutex;

		VulkanDevice& m_Device;

		VulkanMemoryStrategy m_MemoryStrategy;

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


		std::vector<std::shared_ptr<VulkanMemoryPage>> m_MemoryPages;

		


	};








	MAKE_CONSTRUCTED
	class VulkanMemoryPage : public ng::EnableSharedThis<VulkanMemoryPage> {
	public:


		// Make Factory
		static std::shared_ptr<VulkanMemoryPage> Make(VulkanDevice& device, vk::MemoryAllocateInfo allocInfo);

		~VulkanMemoryPage() = default;

		bool CanAllocate(vk::DeviceSize size, vk::DeviceSize alignment);

		std::unique_ptr<VulkanMemoryAllocation> Allocate(vk::DeviceSize size, vk::DeviceSize alignment);

		void Free(std::unique_ptr<VulkanMemoryAllocation> pMemAlloc);

		const VulkanDevice& GetVulkanDevice() const;
		const vk::DeviceMemory GetMemory() const;

		vk::DeviceSize GetUsedSize();
		
		void LockPageMutex();
		void UnlockPageMutex();

	private:

		bool mCanAllocate(vk::DeviceSize size, vk::DeviceSize alignment);

		std::unique_ptr<VulkanMemoryAllocation> mAllocate(vk::DeviceSize size, vk::DeviceSize alignment);

		void mFree(std::unique_ptr<VulkanMemoryAllocation> pMemAlloc);

		vk::DeviceSize mGetUsedSize();

	private:
		VulkanMemoryPage(VulkanDevice& device, vk::MemoryAllocateInfo allocInfo);
		VulkanMemoryPage(const VulkanMemoryPage&) = delete;
		VulkanMemoryPage& operator=(const VulkanMemoryPage&) = delete;

	private:
		friend class VulkanAllocator;

		std::mutex m_Mutex;

		std::unique_ptr<ng::AbstractFreeListAllocator> m_pAllocator;

		vk::UniqueDeviceMemory m_Memory;
		vk::DeviceSize m_Size;

		uint32 m_MemoryTypeIndex;

		VulkanDevice& m_Device;

	};









	ALLOCATOR_CONSTRUCTED
	class VulkanMemoryAllocation {
	public:

		~VulkanMemoryAllocation();

		vk::DeviceSize GetSize();
		vk::DeviceSize GetOffset();

		ng::raw_ptr<VulkanMemoryPage> GetMemoryPage();

	private:
		VulkanMemoryAllocation() = default;
		VulkanMemoryAllocation(std::shared_ptr<VulkanMemoryPage> pMemPage);
		VulkanMemoryAllocation(const VulkanMemoryAllocation&) = delete;
		VulkanMemoryAllocation& operator=(const VulkanMemoryAllocation&) = delete;

	private:
		friend class VulkanMemoryPage;

		std::unique_ptr<ng::AbstractFreeListAllocation> m_pAllocation;
		std::weak_ptr<VulkanMemoryPage> m_pMemoryPage;

	};



}