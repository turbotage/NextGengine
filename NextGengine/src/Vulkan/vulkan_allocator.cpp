#include "vulkan_allocator.h"

#include "vulkan_device.h"
#include "vulkan_utility.h"










// <==================== VULKAN ALLOCATOR ====================================>
//public
ngv::VulkanAllocator::VulkanAllocator(VulkanDevice& device, const VulkanMemoryStrategy& memStrategy)
	: m_Device(device)
{
	m_MemoryStrategy = memStrategy;
}

void ngv::VulkanAllocator::giveBufferAllocation(const std::shared_ptr<VulkanBuffer>& pBuffer, bool lock)
{
	std::unique_lock<std::mutex> lk(m_Mutex, std::defer_lock);
	if (lock) {
		lk.lock();
	}

	std::shared_ptr<VulkanMemoryPage> allocPage;

	//try to find a suitable memory page
	bool foundPage = false;
	for (auto pPage : m_MemoryPages) {
		if (pPage->m_MemoryTypeIndex == pBuffer->m_MemoryTypeIndex) {
			pBuffer->m_pAllocation = pPage->allocate(pBuffer->m_MemoryRequirements.size, pBuffer->m_MemoryRequirements.alignment);
			if (pBuffer->m_pAllocation != nullptr) {
				pBuffer->m_pMemoryPage = pPage;
				foundPage = true;
				allocPage = pPage;
			}
		}
	}

	if (!foundPage) {
		//no suitable page was found, create a new one
		vk::MemoryAllocateInfo allocInfo{};
		allocInfo.allocationSize = m_MemoryStrategy.recommendedPageSize;
		if (pBuffer->m_MemoryRequirements.size > allocInfo.allocationSize) {
			allocInfo.allocationSize = pBuffer->m_MemoryRequirements.size;
		}
		// perhaps we have allocated too much (TODO: consider making this debug check only)
		if (m_MemoryStrategy.maxMemoryUsage < (getUsedMemoryP() + allocInfo.allocationSize)) {
			std::runtime_error("Tried to allocate more memory than allowed by MemorySettings");
		}
		allocInfo.memoryTypeIndex = pBuffer->m_MemoryTypeIndex;

		auto newPage = VulkanMemoryPage::make(m_Device, allocInfo);

		pBuffer->m_pAllocation = newPage->allocate(pBuffer->m_MemoryRequirements.size, pBuffer->m_MemoryRequirements.alignment);
		pBuffer->m_pMemoryPage = newPage;

		allocPage = newPage;

		m_MemoryPages.push_back(newPage);
	}

	//bind the buffer
	m_Device.device().bindBufferMemory(*pBuffer->m_Buffer, *allocPage->m_Memory, pBuffer->m_pAllocation->getOffset());

}

void ngv::VulkanAllocator::giveImageAllocation(const std::shared_ptr<VulkanImage>& pImage, bool lock)
{
	std::unique_lock<std::mutex> lk(m_Mutex, std::defer_lock);
	if (lock) {
		lk.lock();
	}

	std::shared_ptr<VulkanMemoryPage> allocPage;
	bool foundPage = false;
	//try to find a suitable memory page
	for (auto pPage : m_MemoryPages) {
		if (pPage->m_MemoryTypeIndex == pImage->m_MemoryTypeIndex) {
			pImage->m_pAllocation = pPage->allocate(pImage->m_MemoryRequirements.size, pImage->m_MemoryRequirements.alignment);
			if (pImage->m_pAllocation != nullptr) {
				pImage->m_pMemoryPage = pPage;
				foundPage = true;
				allocPage = pPage;
			}
		}
	}

	if (!foundPage) {
		//no suitable page was found, create a new one
		vk::MemoryAllocateInfo allocInfo{};
		allocInfo.allocationSize = m_MemoryStrategy.recommendedPageSize;
		if (pImage->m_MemoryRequirements.size > allocInfo.allocationSize) {
			allocInfo.allocationSize = pImage->m_MemoryRequirements.size;
		}
		// perhaps we have allocated too much (TODO: consider making this debug check only)
		if (m_MemoryStrategy.maxMemoryUsage < (getUsedMemoryP() + allocInfo.allocationSize)) {
			std::runtime_error("Tried to allocate more memory than allowed by MemorySettings");
		}
		allocInfo.memoryTypeIndex = pImage->m_MemoryTypeIndex;

		auto newPage = VulkanMemoryPage::make(m_Device, allocInfo);

		pImage->m_pAllocation = newPage->allocate(pImage->m_MemoryRequirements.size, pImage->m_MemoryRequirements.alignment);
		pImage->m_pMemoryPage = newPage;

		allocPage = newPage;

		m_MemoryPages.push_back(newPage);
	}

	m_Device.device().bindImageMemory(*pImage->m_Image, *allocPage->m_Memory, pImage->m_pAllocation->getOffset());

}

vk::DeviceSize ngv::VulkanAllocator::getUsedMemory(bool lock)
{
	std::unique_lock<std::mutex> lk(m_Mutex, std::defer_lock);
	if (lock) {
		lk.lock();
	}
	return getUsedMemoryP();
}

void ngv::VulkanAllocator::lockAllocatorMutex()
{
	m_Mutex.lock();
}

void ngv::VulkanAllocator::unlockAllocatorMutex()
{
	m_Mutex.unlock();
}


//private
vk::DeviceSize ngv::VulkanAllocator::getUsedMemoryP()
{
	//should never be called by a thread not already locking the allocator mutex
	vk::DeviceSize usedSize = 0;
	for (auto& page : m_MemoryPages) {
		usedSize += page->getUsedSize();
	}
	return usedSize;
}















// <=================== VULKAN MEMORY PAGE ===========================>
//public
std::shared_ptr<ngv::VulkanMemoryPage> ngv::VulkanMemoryPage::make(VulkanDevice& device, vk::MemoryAllocateInfo allocInfo)
{
	return std::shared_ptr<VulkanMemoryPage>(new VulkanMemoryPage(device, allocInfo));
}

bool ngv::VulkanMemoryPage::canAllocate(vk::DeviceSize size, vk::DeviceSize alignment, bool lock) {
	std::unique_lock<std::mutex> lk(m_Mutex, std::defer_lock);
	if (lock) {
		lk.lock();
	}
	return m_pAllocator->canAllocate(size, alignment);
}

std::unique_ptr<ngv::VulkanMemoryAllocation> ngv::VulkanMemoryPage::allocate(vk::DeviceSize size, vk::DeviceSize alignment, bool lock)
{
	std::unique_lock<std::mutex> lk(m_Mutex, std::defer_lock);
	if (lock) {
		lk.lock();
	}
	std::unique_ptr<VulkanMemoryAllocation> ret(new VulkanMemoryAllocation());
	ret->m_pAllocation = m_pAllocator->allocate(size, alignment);
	if (ret->m_pAllocation == nullptr) {
		return nullptr;
	}
	ret->m_pMemoryPage = shared_from_this();
	return ret;
}

void ngv::VulkanMemoryPage::free(std::unique_ptr<VulkanMemoryAllocation> pMemAlloc, bool lock)
{
	std::unique_lock<std::mutex> lk(m_Mutex, std::defer_lock);
	if (lock) {
		lk.lock();
	}
#ifndef NDEBUG
	// an allocated memory-allocation should have a valid pointer to some memory page (check it)
	if (pMemAlloc != nullptr) {
		if (pMemAlloc->m_pMemoryPage.lock().get() != this) {
			std::runtime_error("Tried to free a memory allocation in a memory-page that has not allocated it");
		}
	}
	else {
		std::runtime_error("Tried to free a memory allocation with nullptr page");
	}
#endif

	// reset the mem-page pointer and free from free-list allocator
	pMemAlloc->m_pMemoryPage.reset();
	m_pAllocator->free(std::move(pMemAlloc->m_pAllocation));
}

const ngv::VulkanDevice& ngv::VulkanMemoryPage::vulkanDevice() const
{
	return m_Device;
}

const vk::DeviceMemory ngv::VulkanMemoryPage::memory() const
{
	return *m_Memory;
}

vk::DeviceSize ngv::VulkanMemoryPage::getUsedSize(bool lock)
{
	std::unique_lock<std::mutex> lk(m_Mutex, std::defer_lock);
	if (lock) {
		lk.lock();
	}
	return m_pAllocator->getUsedSize();
}

void ngv::VulkanMemoryPage::lockPageMutex()
{
	m_Mutex.lock();
}

void ngv::VulkanMemoryPage::unlockPageMutex()
{
	m_Mutex.unlock();
}


//private
ngv::VulkanMemoryPage::VulkanMemoryPage(VulkanDevice& device, vk::MemoryAllocateInfo allocInfo)
	: m_Device(device)
{
	m_Memory = device.device().allocateMemoryUnique(allocInfo);
	m_Size = allocInfo.allocationSize;
	m_MemoryTypeIndex = allocInfo.memoryTypeIndex;
	m_pAllocator = ng::AbstractFreeListAllocator::make(allocInfo.allocationSize);
}
















// <=================== VULKAN MEMORY ALLOCATION =======================>
//public

ngv::VulkanMemoryAllocation::~VulkanMemoryAllocation()
{
	if (auto spt = m_pMemoryPage.lock()) {
		spt->lockPageMutex();
		m_pAllocation.reset(); // ~AbstractFreeListAllocation()
		spt->unlockPageMutex();
	}
	m_pMemoryPage.reset();
}

vk::DeviceSize ngv::VulkanMemoryAllocation::getSize()
{
	return m_pAllocation->getSize();
}

vk::DeviceSize ngv::VulkanMemoryAllocation::getOffset()
{
	return m_pAllocation->getOffset();
}

ng::raw_ptr<ngv::VulkanMemoryPage> ngv::VulkanMemoryAllocation::getMemoryPage()
{
	return m_pMemoryPage.lock().get();
}

ngv::VulkanMemoryAllocation::VulkanMemoryAllocation(std::shared_ptr<VulkanMemoryPage> pMemPage)
{
	m_pMemoryPage = pMemPage;
}

//private


