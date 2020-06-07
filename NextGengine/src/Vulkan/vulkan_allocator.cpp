#include "vulkan_allocator.h"

#include "vulkan_device.h"
#include "vulkan_utility.h"

// <=================== VULKAN MEMORY ALLOCATION =======================>
//public
std::unique_ptr<ngv::VulkanMemoryAllocation> ngv::VulkanMemoryAllocation::make(std::shared_ptr<VulkanMemoryPage> pMemPage)
{
	return std::unique_ptr<VulkanMemoryAllocation>(new VulkanMemoryAllocation(pMemPage));
}

vk::DeviceSize ngv::VulkanMemoryAllocation::getSize()
{
	return m_pAllocation->getSize();
}

vk::DeviceSize ngv::VulkanMemoryAllocation::getOffset()
{
	return m_pAllocation->getOffset();
}

ngv::VulkanMemoryAllocation::VulkanMemoryAllocation(std::shared_ptr<VulkanMemoryPage> pMemPage)
{
	m_pMemoryPage = pMemPage;
}

//private


// <=================== VULKAN MEMORY PAGE ===========================>
//public
std::unique_ptr<ngv::VulkanMemoryPage> ngv::VulkanMemoryPage::make(std::shared_ptr<VulkanDevice> pDevice, std::shared_ptr<vk::UniqueDeviceMemory> pMemory, vk::DeviceSize size, uint32 memoryTypeIndex)
{
	return std::unique_ptr<VulkanMemoryPage>(new VulkanMemoryPage(pDevice, pMemory, size, memoryTypeIndex));
}

bool ngv::VulkanMemoryPage::canAllocate(vk::DeviceSize size, vk::DeviceSize alignment) {
	return m_pAllocator->canAllocate(size, alignment);
}

std::unique_ptr<ngv::VulkanMemoryAllocation> ngv::VulkanMemoryPage::allocate(vk::DeviceSize size, vk::DeviceSize alignment)
{
	auto pVAlloc = VulkanMemoryAllocation::make(shared_from_this());

	pVAlloc->m_pAllocation = m_pAllocator->allocate(size, alignment);
	if (pVAlloc->m_pAllocation == nullptr) {
		return nullptr;
	}

	return pVAlloc;
}

bool ngv::VulkanMemoryPage::free(std::shared_ptr<VulkanMemoryAllocation> pMemAlloc)
{
	// an allocated memory-allocation should have a valid pointer to some memory page (check it)
	if (auto spt = pMemAlloc->m_pMemoryPage.lock()) {
		if (spt.get() != this) {
			return false;
		}
	}

	// reset the mem-page pointer and free from free-list allocator
	pMemAlloc->m_pMemoryPage.reset();
	if (!m_pAllocator->free(std::move(pMemAlloc->m_pAllocation))) {
		return false;
	}

	return true;
}

const vk::Device ngv::VulkanMemoryPage::device() const
{
	return m_pDevice->device();
}

const vk::PhysicalDevice ngv::VulkanMemoryPage::physicalDevice() const
{
	return m_pDevice->physicalDevice();
}

const vk::PhysicalDeviceMemoryProperties ngv::VulkanMemoryPage::physicalDeviceMemoryProperties() const
{
	return m_pDevice->physicalDeviceMemoryProperties();
}

const vk::DeviceMemory ngv::VulkanMemoryPage::memory() const
{
	return *(*m_pMemory);
}

//private
ngv::VulkanMemoryPage::VulkanMemoryPage(std::shared_ptr<VulkanDevice> pDevice, std::shared_ptr<vk::UniqueDeviceMemory> pMemory, vk::DeviceSize size, uint32 memoryTypeIndex)
{
	m_pMemory = pMemory; 
	m_Size = size;
	m_MemoryTypeIndex = memoryTypeIndex;
	m_pDevice = pDevice;
	m_pAllocator = ng::AbstractFreeListAllocator::make(size);
}


// <==================== VULKAN ALLOCATOR ====================================>
//public

ngv::VulkanAllocator::VulkanAllocator(std::shared_ptr<VulkanDevice> pDevice, const VulkanMemoryStrategy& memStrategy)
	: m_pDevice(pDevice)
{
	m_MemoryStrategy = memStrategy;
}

bool ngv::VulkanAllocator::giveBufferAllocation(std::raw_ptr<VulkanBuffer> pBuffer)
{
	//try to find a suitable memory page
	for (auto pPage : m_MemoryPages) {
		if (pPage->m_MemoryTypeIndex == pBuffer->m_MemoryTypeIndex) {
			pBuffer->m_pAllocation = pPage->allocate(pBuffer->m_MemoryRequirements.size, pBuffer->m_MemoryRequirements.alignment);
			if (pBuffer->m_pAllocation != nullptr) {
				pBuffer->m_pMemoryPage = pPage;
				return true;
			}
		}
	}

	//no suitable page was found, create a new one
	vk::MemoryAllocateInfo allocInfo{};
	allocInfo.allocationSize = m_MemoryStrategy.recommendedPageSize;
	if (pBuffer->m_MemoryRequirements.size > allocInfo.allocationSize) {
		allocInfo.allocationSize = pBuffer->m_MemoryRequirements.size;
	}
	// perhaps we have allocated too much
	if (m_MemoryStrategy.maxMemoryUsage < (m_UsedMemory + allocInfo.allocationSize)) {
		return false;
	}


	allocInfo.memoryTypeIndex = pBuffer->m_MemoryTypeIndex;

	std::shared_ptr<vk::UniqueDeviceMemory> newMemory = std::make_shared<vk::UniqueDeviceMemory>();
	*newMemory = m_pDevice->device().allocateMemoryUnique(allocInfo);

	std::shared_ptr<VulkanMemoryPage> newPage = VulkanMemoryPage::make(m_pDevice, newMemory, allocInfo.allocationSize, allocInfo.memoryTypeIndex);

	pBuffer->m_pAllocation = newPage->allocate(pBuffer->m_MemoryRequirements.size, pBuffer->m_MemoryRequirements.alignment);
	pBuffer->m_pMemoryPage = newPage;

	return true;
}

bool ngv::VulkanAllocator::looseBufferAllocation(std::raw_ptr<VulkanBuffer> pBuffer)
{
	pBuffer->m_pAllocation.reset();
	pBuffer->m_pMemoryPage.reset();
}

bool ngv::VulkanAllocator::giveImageAllocation(std::raw_ptr<VulkanImage> image)
{
	return false;
}


