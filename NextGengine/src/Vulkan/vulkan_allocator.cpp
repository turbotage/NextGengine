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

void ngv::VulkanAllocator::giveBufferAllocation(VulkanBuffer& buffer)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	mGiveBufferAllocation(buffer);
}

void ngv::VulkanAllocator::giveImageAllocation(VulkanImage& image)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	mGiveImageAllocation(image);
}

vk::DeviceSize ngv::VulkanAllocator::getUsedMemory()
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	return mGetUsedMemory();
}


//private
void ngv::VulkanAllocator::mGiveBufferAllocation(VulkanBuffer& buffer)
{
	std::shared_ptr<VulkanMemoryPage> allocPage;

	//try to find a suitable memory page
	bool foundPage = false;
	for (auto pPage : m_MemoryPages) {
		if (pPage->m_MemoryTypeIndex == buffer.m_MemoryTypeIndex) {
			buffer.m_pAllocation = pPage->allocate(buffer.m_MemoryRequirements.size, buffer.m_MemoryRequirements.alignment);
			if (buffer.m_pAllocation != nullptr) {
				foundPage = true;
				allocPage = pPage;
			}
		}
	}

	if (!foundPage) {
		//no suitable page was found, create a new one
		vk::MemoryAllocateInfo allocInfo{};
		allocInfo.allocationSize = m_MemoryStrategy.recommendedPageSize;
		if (buffer.m_MemoryRequirements.size > allocInfo.allocationSize) {
			allocInfo.allocationSize = buffer.m_MemoryRequirements.size;
		}
		// perhaps we have allocated too much (TODO: consider making this debug check only)
		if (m_MemoryStrategy.maxMemoryUsage < (mGetUsedMemory() + allocInfo.allocationSize)) {
			std::runtime_error("Tried to allocate more memory than allowed by MemorySettings");
		}
		allocInfo.memoryTypeIndex = buffer.m_MemoryTypeIndex;

		auto newPage = VulkanMemoryPage::make(m_Device, allocInfo);

		buffer.m_pAllocation = newPage->allocate(buffer.m_MemoryRequirements.size, buffer.m_MemoryRequirements.alignment);

		allocPage = newPage;

		m_MemoryPages.push_back(newPage);
	}

	//bind the buffer
	m_Device.device().bindBufferMemory(*buffer.m_Buffer, *allocPage->m_Memory, buffer.m_pAllocation->getOffset());

}

void ngv::VulkanAllocator::mGiveImageAllocation(VulkanImage& image)
{
	std::shared_ptr<VulkanMemoryPage> allocPage;
	bool foundPage = false;
	//try to find a suitable memory page
	for (auto pPage : m_MemoryPages) {
		if (pPage->m_MemoryTypeIndex == image.m_MemoryTypeIndex) {
			image.m_pAllocation = pPage->allocate(image.m_MemoryRequirements.size, image.m_MemoryRequirements.alignment);
			if (image.m_pAllocation != nullptr) {
				foundPage = true;
				allocPage = pPage;
			}
		}
	}

	if (!foundPage) {
		//no suitable page was found, create a new one
		vk::MemoryAllocateInfo allocInfo{};
		allocInfo.allocationSize = m_MemoryStrategy.recommendedPageSize;
		if (image.m_MemoryRequirements.size > allocInfo.allocationSize) {
			allocInfo.allocationSize = image.m_MemoryRequirements.size;
		}
		// perhaps we have allocated too much (TODO: consider making this debug check only)
		if (m_MemoryStrategy.maxMemoryUsage < (mGetUsedMemory() + allocInfo.allocationSize)) {
			std::runtime_error("Tried to allocate more memory than allowed by MemorySettings");
		}
		allocInfo.memoryTypeIndex = image.m_MemoryTypeIndex;

		auto newPage = VulkanMemoryPage::make(m_Device, allocInfo);

		image.m_pAllocation = newPage->allocate(image.m_MemoryRequirements.size, image.m_MemoryRequirements.alignment);

		allocPage = newPage;

		m_MemoryPages.push_back(newPage);
	}

	m_Device.device().bindImageMemory(*image.m_Image, *allocPage->m_Memory, image.m_pAllocation->getOffset());
}

vk::DeviceSize ngv::VulkanAllocator::mGetUsedMemory()
{
	//should never be called by a thread not already locking the allocator mutex
	vk::DeviceSize usedSize = 0;
	for (auto& page : m_MemoryPages) {
		usedSize += page->getUsedSize();
	}
	return usedSize;
}















// <=================== VULKAN MEMORY PAGE ===========================>
// PUBLIC
std::shared_ptr<ngv::VulkanMemoryPage> ngv::VulkanMemoryPage::make(VulkanDevice& device, vk::MemoryAllocateInfo allocInfo)
{
	return std::shared_ptr<VulkanMemoryPage>(new VulkanMemoryPage(device, allocInfo));
}

bool ngv::VulkanMemoryPage::canAllocate(vk::DeviceSize size, vk::DeviceSize alignment) {
	std::lock_guard<std::mutex> lock(m_Mutex);
	return mCanAllocate(size, alignment);
}

std::unique_ptr<ngv::VulkanMemoryAllocation> ngv::VulkanMemoryPage::allocate(vk::DeviceSize size, vk::DeviceSize alignment)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	return mAllocate(size, alignment);
}

void ngv::VulkanMemoryPage::free(std::unique_ptr<VulkanMemoryAllocation> pMemAlloc)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	mFree(std::move(pMemAlloc));
}

const ngv::VulkanDevice& ngv::VulkanMemoryPage::vulkanDevice() const
{
	return m_Device;
}

const vk::DeviceMemory ngv::VulkanMemoryPage::memory() const
{
	return *m_Memory;
}

vk::DeviceSize ngv::VulkanMemoryPage::getUsedSize()
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	return mGetUsedSize();
}

void ngv::VulkanMemoryPage::lockPageMutex()
{
	m_Mutex.lock();
}

void ngv::VulkanMemoryPage::unlockPageMutex()
{
	m_Mutex.unlock();
}

// PRIVATE
bool ngv::VulkanMemoryPage::mCanAllocate(vk::DeviceSize size, vk::DeviceSize alignment)
{
	return m_pAllocator->canAllocate(size, alignment);
}

std::unique_ptr<ngv::VulkanMemoryAllocation> ngv::VulkanMemoryPage::mAllocate(vk::DeviceSize size, vk::DeviceSize alignment)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	std::unique_ptr<VulkanMemoryAllocation> ret(new VulkanMemoryAllocation());
	ret->m_pAllocation = m_pAllocator->allocate(size, alignment);
	if (ret->m_pAllocation == nullptr) {
		return nullptr;
	}
	ret->m_pMemoryPage = shared_from_this();
	return ret;
}

void ngv::VulkanMemoryPage::mFree(std::unique_ptr<VulkanMemoryAllocation> pMemAlloc)
{
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

vk::DeviceSize ngv::VulkanMemoryPage::mGetUsedSize()
{
	return m_pAllocator->getUsedSize();
}

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


