#include "vulkan_allocator.h"

#include "vulkan_storage.h"

vk::Device ngv::VulkanAllocator::getDevice()
{
	return m_Device;
}

vk::DeviceSize ngv::VulkanAllocator::getAlignedSize(vk::DeviceSize size, vk::DeviceSize alignment)
{
	if (size % alignment != 0) {
		size += alignment - (size % alignment);
	}
	return size;
}



void ngv::VulkanAllocator::init(VulkanAllocatorCreateInfo createInfo)
{

}

bool ngv::VulkanAllocator::giveAllocatorHints(VulkanAllocatorHints hints)
{
	/*
	bool canManageHints = true;
	m_CurrentImageHints = hints.imageHints;
	m_AvailableHintImages.resize(m_CurrentImageHints.size());
	uint32 i = 0;
	for (auto& imageHint : m_CurrentImageHints) {
		uint32 availableImages = 0;
		if (std::get<1>(imageHint) & vk::MemoryPropertyFlagBits::eDeviceLocal) {
			for (auto& imagePage : m_DeviceImages) {
				if (std::get<1>(imageHint) == imagePage.memoryProperties) {
					if (std::get<0>(imageHint) == imagePage.imageCreateInfo) {
						availableImages += imagePage.allocator.getNumberOfBlocks();
					}
				}
			}
		}
		else {
			for (auto& imagePage : m_HostImages) {
				if (std::get<1>(imageHint) == imagePage.memoryProperties) {
					if (std::get<0>(imageHint) == imagePage.imageCreateInfo) {
						availableImages += imagePage.allocator.getNumberOfBlocks();
					}
				}
			}
		}
		if (availableImages < std::get<2>(imageHint)) {
			canManageHints = false;
			m_AvailableHintImages[i] = availableImages;
		}
		i = i + 1; // xD
	}
	*/
}



/* PUBLIC IMAGE FUNCTIONS */
bool ngv::VulkanAllocator::giveImagesAllocations(const std::vector<std::reference_wrapper<VulkanImage>>& images)
{
	for (auto& image : images) {
		giveImageAllocation(image);
	}
}

bool ngv::VulkanAllocator::createImage(const VulkanImageCreateInfo& createInfo, VulkanImage& image, bool allocate)
{
	int a = 0;

	image.m_CreateInfo = createInfo;
	image.m_pAllocation.reset();
	image.m_pImages->erase(&image);
	image.m_pImages = std::make_shared<VulkanImage*>();
	image.m_pImages->insert(&image);
	if (allocate) {
		return giveImageAllocation(image);
	}
	return true;
}

bool ngv::VulkanAllocator::giveImageAllocation(VulkanImage& image)
{
	vk::Result result;
	bool inDevice = true;

	std::shared_ptr<std::list<VulkanImagePage>> foundPageList = nullptr;
	bool foundPageCanAllocate = false;
	// memory property flags
	std::list<std::shared_ptr<std::list<VulkanImagePage>>>& searchImages = m_DeviceImages;
	if (!(image.m_CreateInfo.memoryPropertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal)) {
		inDevice = false;
		searchImages = m_HostImages;
	}
	for (auto& pageList : searchImages) {
		if (pageList->begin()->isSuitable(image.m_CreateInfo)) {
			for (auto& page : *pageList) {
				foundPageList = pageList;
				if (page.hasFreeAllocations()) {
					foundPageCanAllocate = true;
					break;
				}
			}
			break;
		}
	}

	if (foundPageList != nullptr) { //there are similar image pages
		if (foundPageCanAllocate) { //there are similar image pages that can allocate for this image
			auto it = findImagePage(image.m_CreateInfo, foundPageList, result);
			image.m_pAllocation = it->getAllocation();
			return true;
		}
		else { //there are similar image pages but none that can allocate
			auto it = createImagePage(image.m_CreateInfo, foundPageList, result, inDevice);
			if (result != vk::Result::eSuccess) {
				return false;
			}
			image.m_pAllocation = it->getAllocation();
			return true;
		}
	}
	else { // there were no similar image pages
		auto it = createImagePage(image.m_CreateInfo, nullptr, result, inDevice);
		if (result != vk::Result::eSuccess) {
			return false;
		}
		image.m_pAllocation = it->getAllocation();

	}

}

















/* PRIVATE IMAGES FUNCTIONS */
std::list<ngv::VulkanImagePage>::iterator ngv::VulkanAllocator::createImagePage(const VulkanImageCreateInfo& createInfo, std::shared_ptr<std::list<VulkanImagePage>> pImageTypeList, vk::Result& result, bool inDevice)
{
	if (pImageTypeList != nullptr) {
		pImageTypeList->emplace_back();
		auto newImagePage = std::prev(pImageTypeList->end());
		auto& firstOfType = pImageTypeList->begin();

		vk::MemoryAllocateInfo mai{};
		mai.allocationSize = firstOfType->memoryPage->size;
		mai.memoryTypeIndex = firstOfType->memoryPage->memoryTypeIndex;

		std::shared_ptr<VulkanMemoryPage> memPage = std::make_shared<VulkanMemoryPage>();
		memPage->memory = m_Device.allocateMemoryUnique(mai);
		memPage->size = firstOfType->memoryPage->size;
		memPage->memoryPropertyFlags = firstOfType->memoryPage->memoryPropertyFlags;
		memPage->memoryTypeIndex = firstOfType->memoryPage->memoryTypeIndex;

		// setup pool allocator info
		VulkanPoolAllocatorCreateInfo poolInfo;
		poolInfo.blockSize = firstOfType->poolAllocator.getBlockSize();
		poolInfo.nBlocks = firstOfType->poolAllocator.getNumberOfBlocks();
		
		// set memory page
		newImagePage->pVulkanAllocator = this;
		newImagePage->poolAllocator.init(poolInfo);
		newImagePage->memoryPage = memPage;
		newImagePage->imageCreateInfo = firstOfType->imageCreateInfo;
		newImagePage->memoryRequirements = firstOfType->memoryRequirements;

		//
		for (int i = 0; i < poolInfo.nBlocks; ++i) {
			std::shared_ptr<VulkanImageAllocation> tempImageAllocation = std::make_shared<VulkanImageAllocation>();
			tempImageAllocation->pImagePage = &(*newImagePage);
			tempImageAllocation->poolAllocation = newImagePage->poolAllocator.allocate();
			tempImageAllocation->image = m_Device.createImageUnique(createInfo.imageCreateInfo);
			newImagePage->freeImageAllocations.insert(tempImageAllocation);
		}

	}
	else {
		vk::UniqueImage tempImage = m_Device.createImageUnique(createInfo.imageCreateInfo);


		vk::MemoryRequirements memReqs = m_Device.getImageMemoryRequirements(tempImage.get());

		std::shared_ptr<VulkanMemoryPage> memPage = std::make_shared<VulkanMemoryPage>();

		vk::DeviceSize blockSize = getAlignedSize(memReqs.size, memReqs.alignment);

		uint32 numberOfBlocks;
		if (blockSize <= m_MemoryStrategy.smallCapImageSize) { numberOfBlocks = m_MemoryStrategy.smallCapImagesPerPool; }
		else if (blockSize <= m_MemoryStrategy.mediumCapImageSize) { numberOfBlocks = m_MemoryStrategy.mediumCapImagesPerPool; }
		else if (blockSize <= m_MemoryStrategy.largeCapImageSize) { numberOfBlocks = m_MemoryStrategy.largeCapImagesPerPool; }
		else {
			//should have been a dedicated image
			result = vk::Result::eErrorFormatNotSupported;
			return std::list<ngv::VulkanImagePage>::iterator();
		}
		vk::DeviceSize memorySize = blockSize * numberOfBlocks;

		vk::MemoryAllocateInfo allocInfo{};
		allocInfo.allocationSize = memorySize;
		allocInfo.memoryTypeIndex = m_Context->getMemoryType(memReqs.memoryTypeBits, createInfo.memoryPropertyFlags);


		memPage->memory = m_Device.allocateMemoryUnique(allocInfo);
		memPage->size = memorySize;
		memPage->memoryPropertyFlags = createInfo.memoryPropertyFlags;
		memPage->memoryTypeIndex = allocInfo.memoryTypeIndex;

		std::shared_ptr<std::list<VulkanImagePage>> pImageTypeList = std::make_shared<std::list<VulkanImagePage>>();
		pImageTypeList->emplace_back();

		auto it = std::prev(pImageTypeList->end());

		VulkanPoolAllocatorCreateInfo poolInfo;
		poolInfo.blockSize = blockSize;
		poolInfo.nBlocks = numberOfBlocks;

		it->pVulkanAllocator = this;
		it->poolAllocator.init(poolInfo);
		it->memoryPage = memPage;
		it->imageCreateInfo = createInfo.imageCreateInfo;
		it->memoryRequirements = memReqs;

		//create all images
		for (int i = 0; i < numberOfBlocks; ++i) {
			std::shared_ptr<VulkanImageAllocation> tempImageAllocation = std::make_shared<VulkanImageAllocation>();
			tempImageAllocation->pImagePage = &(*it);
			tempImageAllocation->poolAllocation = it->poolAllocator.allocate();
			tempImageAllocation->image = m_Device.createImageUnique(createInfo.imageCreateInfo);
			it->freeImageAllocations.insert(tempImageAllocation);
		}

		if (inDevice) {
			m_DeviceImages.push_back(pImageTypeList);
		}
		else {
			m_HostImages.push_back(pImageTypeList);
		}

	}


}

std::list<ngv::VulkanImagePage>::iterator ngv::VulkanAllocator::findImagePage(const VulkanImageCreateInfo& createInfo, std::shared_ptr<std::list<VulkanImagePage>> pPageList, vk::Result& result)
{
	// TODO: choose the one that already has the most allocations used
	auto bestImagePage = pPageList->begin();
	for (auto it = pPageList->begin(); it != pPageList->end(); it++) {
		if (it->poolAllocator.hasFreeBlocks()) {
			
		}
	}
	return bestImagePage;
}














/* Pool Allocator */

void ngv::VulkanPoolAllocator::init(VulkanPoolAllocatorCreateInfo createInfo)
{
	m_NumberOfBlocks = createInfo.nBlocks;
	m_BlockSize = createInfo.blockSize;

	m_Size = m_NumberOfBlocks * m_BlockSize;

	m_FreeBlocks.reserve(m_NumberOfBlocks);
	m_FreeBlocks.resize(m_NumberOfBlocks);

	for (uint32 i = 0; i < m_NumberOfBlocks; ++i) {
		m_FreeBlocks[i] = i;
	}
}

bool ngv::VulkanPoolAllocator::hasFreeBlocks()
{
	return !m_FreeBlocks.empty();
}

ngv::VulkanPoolAllocation ngv::VulkanPoolAllocator::allocate()
{
	ngv::VulkanPoolAllocation ret;

	ret.offset = m_FreeBlocks.back() * m_BlockSize;
	ret.usedSize = m_BlockSize;

	m_FreeBlocks.pop_back();

	return ret;
}

bool ngv::VulkanPoolAllocator::free(VulkanPoolAllocation allocation)
{
	uint32 toFree = allocation.offset / m_BlockSize;
	m_FreeBlocks.push_back(toFree);
	return VK_SUCCESS;
}

uint32 ngv::VulkanPoolAllocator::getNumberOfBlocks()
{
	return m_NumberOfBlocks;
}

vk::DeviceSize ngv::VulkanPoolAllocator::getBlockSize()
{
	return m_BlockSize;
}

vk::DeviceSize ngv::VulkanPoolAllocator::getSize()
{
	return m_Size;
}

/* Free Allocator */

void ngv::VulkanFreeAllocator::init(VulkanFreeAllocatorCreateInfo createInfo)
{
	m_NumberOfBlocks = createInfo.size / createInfo.blockSize; //size must always be a multiple of blockSize

	BlockCollection newBlockCol;
	newBlockCol.startBlock = 0;
	newBlockCol.nBlocks = m_NumberOfBlocks;

	m_FreeBlocks.push_back(newBlockCol);
}

bool ngv::VulkanFreeAllocator::enoughSpaceWithoutDefrag(vk::DeviceSize requiredSize)
{
	for (auto it = m_FreeBlocks.begin(); it != m_FreeBlocks.end(); it++) {
		if (it->nBlocks * m_BlockSize >= requiredSize) {
			return true;
		}
	}
	return false;
}

bool ngv::VulkanFreeAllocator::enoughSpace(vk::DeviceSize requiredSize)
{
	VkDeviceSize totalSize = 0;
	for (auto it = m_FreeBlocks.begin(); it != m_FreeBlocks.end(); it++) {
		totalSize += it->nBlocks * m_BlockSize;
	}
	if (totalSize >= requiredSize) {
		return true;
	}
	return false;
}

/*  should never be called without knowing it can allocate first (check with enoughSpaceWithoutDefrag first) */
ngv::VulkanFreeAllocation ngv::VulkanFreeAllocator::allocate(vk::DeviceSize requiredSize)
{
	std::list<BlockCollection>::iterator smallestCollection = m_FreeBlocks.end();

	uint32 requiredNumOfBlocks = (uint32)ceil(requiredSize / m_BlockSize);

	//first find a block that is big enough
	for (auto it = m_FreeBlocks.begin(); it != m_FreeBlocks.end(); it++) {
		if (it->nBlocks >= requiredNumOfBlocks) {
			smallestCollection = it;
			break;
		}
	}

	//find more suitable match, (smallest match)
	for (auto it = ++smallestCollection; it != m_FreeBlocks.end(); it++) {
		if (it->nBlocks >= requiredNumOfBlocks) {
			if (it->nBlocks < smallestCollection->nBlocks) {
				smallestCollection = it;
			}
		}
	}

	ngv::VulkanFreeAllocation ret;
	ret.offset = smallestCollection->startBlock * m_BlockSize;
	ret.usedSize = requiredNumOfBlocks * m_BlockSize;

	smallestCollection->startBlock += +requiredNumOfBlocks;
	smallestCollection->nBlocks -= requiredNumOfBlocks;

	return ret;
}

bool ngv::VulkanFreeAllocator::free(VulkanFreeAllocation allocation)
{
	// possible situations
	/*
	1)  BLOCK TO FREE | FREE BLOCK | ...
	2)  BLOCK TO FREE | USED BLOCK | ...
	3)  ... | FREE BLOCK | BLOCK TO FREE
	4)  ... | USED BLOCK | BLOCK TO FREE
	5)  FREE BLOCK | BLOCK TO FREE | FREE BLOCK
	6)  FREE BLOCK | BLOCK TO FREE | USED BLOCK
	7)  USED BLOCK | BLOCK TO FREE | USED BLOCK
	8)  USED BLOCK | BLOCK TO FREE | FREE BLOCK
	*/

	uint32 startBlock = allocation.offset / m_BlockSize;
	uint32 nBlocks = allocation.usedSize / m_BlockSize;

	bool prevIsFree = false;
	std::list<BlockCollection>::iterator prevBC = m_FreeBlocks.end();
	bool nextIsFree = false;
	std::list<BlockCollection>::iterator nextBC = m_FreeBlocks.end();

	//see if prev block is a free blockcollection
	for (auto it = m_FreeBlocks.begin(); it != m_FreeBlocks.end(); it++) {
		if (startBlock == (it->startBlock + it->nBlocks)) {
			prevBC = it;
			prevIsFree = true;
		}
	}

	//see if next block is a free blockcollection
	for (auto it = m_FreeBlocks.begin(); it != m_FreeBlocks.end(); it++) {
		if ((startBlock + nBlocks) == it->startBlock) {
			nextBC = it;
			nextIsFree = true;
		}
	}

	// 1) || 2)
	if (startBlock == 0) {
		if (nextIsFree) {  // 1)
			nextBC->startBlock = 0;
			nextBC->nBlocks = nextBC->nBlocks + nBlocks;
		}
		else {   // 2)
			BlockCollection newFreeBlock;
			newFreeBlock.startBlock = 0;
			newFreeBlock.nBlocks = nBlocks;
			m_FreeBlocks.push_front(newFreeBlock);
		}
		return true; //return VK_SUCCESS;
	}

	// 5)
	if (prevIsFree && nextIsFree) {
		prevBC->nBlocks = prevBC->nBlocks + nBlocks + nextBC->nBlocks;
		m_FreeBlocks.erase(nextBC);
		return true; //return VK_SUCCESS;
	}

	// 6) & 3)
	if (prevIsFree && !nextIsFree) {
		prevBC->nBlocks = prevBC->nBlocks + nBlocks;
		return true; //return VK_SUCCESS;
	}

	// 7) & 4)
	if (!prevIsFree && !nextIsFree) {
		BlockCollection newFreeBlock;
		newFreeBlock.startBlock = startBlock;
		newFreeBlock.nBlocks = nBlocks;
		m_FreeBlocks.push_front(newFreeBlock);
		return true; //return VK_SUCCESS;
	}

	// 8)
	if (!prevIsFree && nextIsFree) {
		nextBC->startBlock = startBlock;
		nextBC->nBlocks = nextBC->nBlocks + nBlocks;
		return true; //return VK_SUCCESS;
	}

	return false;
}


/* Memory Paging */

ngv::VulkanMemoryPage::VulkanMemoryPage()
{
}

ngv::VulkanMemoryPage::~VulkanMemoryPage()
{
}

ngv::VulkanImagePage::VulkanImagePage()
{
}

bool ngv::VulkanImagePage::hasFreeAllocations()
{
	return !freeImageAllocations.empty();
}

bool ngv::VulkanImagePage::isSuitable(const VulkanImageCreateInfo& createInfo)
{
	return (imageCreateInfo == createInfo.imageCreateInfo) && (memoryPage->memoryPropertyFlags == createInfo.memoryPropertyFlags);
}

std::shared_ptr<ngv::VulkanImageAllocation> ngv::VulkanImagePage::getAllocation()
{
	auto it = freeImageAllocations.begin();
	std::shared_ptr<ngv::VulkanImageAllocation> ret = *it;
	freeImageAllocations.erase(it);
	usedImageAllocations.insert(ret);
	return ret;
}

void ngv::VulkanImagePage::freeAllocation(std::shared_ptr<ngv::VulkanImageAllocation> toFree)
{
	if (usedImageAllocations.find(toFree) == usedImageAllocations.end()) {
		//this should not happen
		return;
	}
	usedImageAllocations.erase(toFree);
	freeImageAllocations.insert(toFree);
}

uint32 ngv::VulkanImagePage::getNumFreeAllocations()
{
	return freeImageAllocations.size();
}

ngv::VulkanBufferPage::VulkanBufferPage()
{
	memoryPage = std::make_shared<VulkanMemoryPage>();
}

ngv::VulkanUniformBufferPage::VulkanUniformBufferPage()
{
	memoryPage = std::make_shared<VulkanMemoryPage>();
}

ngv::VulkanImageAllocation::VulkanImageAllocation()
{
}