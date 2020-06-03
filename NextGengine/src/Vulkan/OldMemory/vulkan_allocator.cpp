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

vk::DeviceSize ngv::VulkanAllocator::getRecommendedPageSize(vk::BufferUsageFlags usageFlags)
{
	if (usageFlags & vk::BufferUsageFlagBits::eVertexBuffer) {
		return 256 * 1024 * 1024;
	}
	else if (usageFlags & vk::BufferUsageFlagBits::eRayTracingKHR) {
		return 256 * 1024 * 1024;
	}
	
	return 64 * 1024 * 1024;
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
	image.m_CreateInfo = createInfo;
	image.m_pAllocation.reset();
	image.m_pImages->erase(&image);
	image.m_pImages = std::make_shared<std::set<VulkanImage*>>();
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
	std::list<std::shared_ptr<std::list<VulkanImagePage>>>& searchPageLists = m_DeviceImagePageLists;
	if (!(image.m_CreateInfo.memoryPropertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal)) {
		inDevice = false;
		searchPageLists = m_HostImagePageLists;
	}
	for (auto& pageList : searchPageLists) {
		if (pageList->begin()->isSuitable(image.m_CreateInfo)) {
			foundPageList = pageList;
			for (auto& page : *pageList) {
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
			auto temp = it->getAllocation();
			temp->images.insert(&image);
			image.m_pAllocation = temp;
			return true;
		}
		else { //there are similar image pages but none that can allocate
			auto it = createImagePage(image.m_CreateInfo, foundPageList, result, inDevice);
			if (result != vk::Result::eSuccess) {
				return false;
			}
			auto temp = it->getAllocation();
			temp->images.insert(&image);
			image.m_pAllocation = temp;
			return true;
		}
	}
	else { // there were no similar image pages
		auto it = createImagePage(image.m_CreateInfo, nullptr, result, inDevice);
		if (result != vk::Result::eSuccess) {
			return false;
		}
		auto temp = it->getAllocation();
		temp->images.insert(&image);
		image.m_pAllocation = temp;
		return true;
	}

}

bool ngv::VulkanAllocator::giveBuffersAllocations(const std::vector<std::reference_wrapper<VulkanBuffer>>& buffers)
{
	for (auto& buffer : buffers) {
		giveBufferAllocation(buffer);
	}
}

bool ngv::VulkanAllocator::createBuffer(const VulkanBufferCreateInfo& createInfo, VulkanBuffer& buffer, bool allocate)
{
	buffer.m_CreateInfo = createInfo;
	buffer.m_pAllocation.reset();
	buffer.m_pBuffers->erase(&buffer);
	buffer.m_pBuffers = std::make_shared<std::set<VulkanBuffer*>>();
	buffer.m_pBuffers->insert(&buffer);
	if (allocate) {
		return giveBufferAllocation(buffer);
	}
	return true;
}

bool ngv::VulkanAllocator::giveBufferAllocation(VulkanBuffer& buffer)
{
	vk::Result result;
	bool inDevice = true;

	bool foundPage = false;
	VulkanBufferPage* pFoundPage;

	std::list<VulkanBufferPage>& searchList = m_DeviceBufferPages;
	if (!(buffer.m_CreateInfo.memoryPropertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal)) {
		inDevice = false;
		searchList = m_HostBufferPages;
	}
	for (auto& page : searchList) {
		if (page.isSuitable(buffer.m_CreateInfo)) {
			if (page.enoughSpaceWithoutDefrag(buffer.m_CreateInfo.bufferCreateInfo.size)) {
				foundPage = true;
				pFoundPage = &page;
			}
		}
	}

	if (foundPage) {
		auto temp = pFoundPage->getAllocation(buffer.m_CreateInfo.bufferCreateInfo.size);
		temp->buffers.insert(&buffer);
		buffer.m_pAllocation = temp;
		return true;
	}
	else {
		auto it = createBufferPage(buffer.m_CreateInfo, result,)
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
			m_DeviceImagePageLists.push_back(pImageTypeList);
		}
		else {
			m_HostImagePageLists.push_back(pImageTypeList);
		}

	}


}
vk::DeviceSize
std::list<ngv::VulkanBufferPage>::iterator ngv::VulkanAllocator::createBufferPage(const VulkanBufferCreateInfo& createInfo, vk::Result& result, bool inDevice)
{
	std::list<VulkanBufferPage>::iterator newBufPageIt;
	if (createInfo.memoryPropertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal) {
		m_DeviceBufferPages.emplace_back();
		newBufPageIt = std::prev(m_DeviceBufferPages.end());
	}
	else {
		m_HostBufferPages.emplace_back();
		newBufPageIt = std::prev(m_HostBufferPages.end());
	}


	vk::BufferCreateInfo bufCreateInfo;
	bufCreateInfo = createInfo.bufferCreateInfo;
	bufCreateInfo.size = getRecommendedPageSize(bufCreateInfo.usage);
	bufCreateInfo.size = std::ceil(bufCreateInfo.size / m_MemoryStrategy.freeAllocatorBlockSize) * m_MemoryStrategy.freeAllocatorBlockSize;

	newBufPageIt->buffer = m_Device.createBufferUnique(bufCreateInfo);

	vk::MemoryRequirements memReqs;
	m_Device.getBufferMemoryRequirements(newBufPageIt->buffer.get(), &memReqs);

	newBufPageIt->memoryPage = std::make_shared<VulkanMemoryPage>();

	
	vk::MemoryAllocateInfo allocInfo{};
	allocInfo.allocationSize = memReqs.size;
	allocInfo.memoryTypeIndex = m_Context->getMemoryType(memReqs.memoryTypeBits, createInfo.memoryPropertyFlags);

	newBufPageIt->memoryPage->memory = m_Device.allocateMemoryUnique(allocInfo);

	newBufPageIt->createFlags = createInfo.bufferCreateInfo.flags;
	newBufPageIt->usageFlags = createInfo.bufferCreateInfo.usage;
	newBufPageIt->memoryPage->memoryPropertyFlags = createInfo.memoryPropertyFlags;
	newBufPageIt->memoryPage->memoryTypeIndex = allocInfo.memoryTypeIndex;
	newBufPageIt->memoryPage->size = allocInfo.allocationSize;

	ngv::VulkanFreeAllocatorCreateInfo allocatorCreateInfo{};
	allocatorCreateInfo.blockSize = m_MemoryStrategy.freeAllocatorBlockSize;
	allocatorCreateInfo.size = bufCreateInfo.size;

	newBufPageIt->allocator.init(allocatorCreateInfo);



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







/* Memory Paging */
/* MEMORY PAGE */

ngv::VulkanMemoryPage::VulkanMemoryPage() {}

ngv::VulkanMemoryPage::~VulkanMemoryPage() {}

/* IMAGE */

ngv::VulkanImageAllocation::VulkanImageAllocation()
{

}

ngv::VulkanImagePage::VulkanImagePage() {}

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

/* BUFFER */

ngv::VulkanBufferAllocation::VulkanBufferAllocation() {}

ngv::VulkanBufferPage::VulkanBufferPage() {}

std::shared_ptr<ngv::VulkanBufferAllocation> ngv::VulkanBufferPage::getAllocation(vk::DeviceSize requiredSize)
{
	auto ret = std::make_shared<VulkanBufferAllocation>();
	ret->pBufferPage = this;
	ret->freeAllocation = this->allocator.allocate(requiredSize);

}

bool ngv::VulkanBufferPage::isSuitable(const VulkanBufferCreateInfo& createInfo)
{
	if ((createInfo.memoryPropertyFlags == memoryPage->memoryPropertyFlags) &&
		(createInfo.bufferCreateInfo.flags == createFlags) &&
		(createInfo.bufferCreateInfo.usage == usageFlags) &&
		(createInfo.bufferCreateInfo.sharingMode == vk::SharingMode::eExclusive)) //this should be changed for later
	{
		return true;
	}
	return false;
}

bool ngv::VulkanBufferPage::enoughSpaceWithoutDefrag(vk::DeviceSize requiredSize)
{
	return allocator.enoughSpaceWithoutDefrag(requiredSize);
}

bool ngv::VulkanBufferPage::enoughSpace(vk::DeviceSize requiredSize)
{
	return allocator.enoughSpace(requiredSize);
}












ngv::VulkanUniformBufferPage::VulkanUniformBufferPage()
{
}
