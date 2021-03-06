#include "vulkan_storage.h"



#include "vulkan_allocator.h"
#include "vulkan_utility.h"
#include "vulkan_device.h"





// <==================== VULKAN BUFFER ==========================>

std::unique_ptr<ngv::VulkanBuffer> ngv::VulkanBuffer::Make(const VulkanDevice& device, const vk::BufferCreateInfo& info, bool hostBuffer)
{
	return std::unique_ptr<VulkanBuffer>(new VulkanBuffer(device, info, hostBuffer));
}

vk::Buffer ngv::VulkanBuffer::Buffer() const
{
	return *m_Buffer;
}

const vk::BufferCreateInfo ngv::VulkanBuffer::GetBufferCreateInfo() const
{
	return m_BufferCreateInfo;
}

const vk::MemoryPropertyFlags ngv::VulkanBuffer::GetMemoryPropertyFlags() const
{
	return m_MemoryPropertyFlags;
}

void* ngv::VulkanBuffer::Map()
{
	auto spt = m_pAllocation->getMemoryPage();
#ifndef NDEBUG
	if (spt == nullptr) {
		std::runtime_error("Tried to map buffer without allocation");
	}
#endif

	const VulkanDevice& vulkanDevice = spt->vulkanDevice();
	vk::Device device = vulkanDevice.device();
	/*
	vk::DeviceSize atomSize = vulkanDevice.physicalDeviceLimits().nonCoherentAtomSize;
	vk::DeviceSize mapSize = (vk::DeviceSize)std::ceil(m_BufferCreateInfo.size / atomSize) * atomSize;
	if (mapSize > m_BufferCreateInfo.size) {
		mapSize = VK_WHOLE_SIZE;
	}
	*/

	return device.mapMemory(spt->memory(), m_pAllocation->getOffset(), m_BufferCreateInfo.size);
}

void* ngv::VulkanBuffer::Map(uint64 offset, uint64 size) {
	auto spt = m_pAllocation->getMemoryPage();
#ifndef NDEBUG
	if (spt == nullptr) {
		std::runtime_error("Tried to map buffer without allocation");
	}
#endif

	const VulkanDevice& vulkanDevice = spt->vulkanDevice();
	vk::Device device = vulkanDevice.device();
	/*
	vk::DeviceSize atomSize = vulkanDevice.physicalDeviceLimits().nonCoherentAtomSize;
	vk::DeviceSize mapSize = (vk::DeviceSize)std::ceil(m_BufferCreateInfo.size / atomSize) * atomSize;
	if (mapSize > m_BufferCreateInfo.size) {
		mapSize = VK_WHOLE_SIZE;
	}
	*/

	spt->lockPageMutex();
	return device.mapMemory(spt->memory(), m_pAllocation->getOffset() + offset, size);
}

void ngv::VulkanBuffer::Unmap()
{
	auto spt = m_pAllocation->getMemoryPage();
	spt->vulkanDevice().device().unmapMemory(spt->memory());
	spt->unlockPageMutex();
}

void ngv::VulkanBuffer::UpdateLocal(const void* value, vk::DeviceSize size) const
{
	auto spt = m_pAllocation->getMemoryPage();
#ifndef NDEBUG
	if (size == 0) {
		std::runtime_error("Tried to update buffer with size 0");
	}
	if (spt == nullptr) {
		std::runtime_error("Tried to updateLocal on buffer missing allocation");
	}
#endif
	const VulkanDevice& vulkanDevice = spt->vulkanDevice();
	vk::Device device = vulkanDevice.device();

	vk::DeviceMemory mem = spt->memory();
	vk::DeviceSize offset = m_pAllocation->getOffset();

	spt->lockPageMutex();
	void* ptr = device.mapMemory(mem, offset, size);
	memcpy(ptr, value, (size_t)size);

	/* WE ALWAYS USE COHERENT MEMORY
	vk::DeviceSize atomSize = vulkanDevice.physicalDeviceLimits().nonCoherentAtomSize;
	vk::DeviceSize mapSize = (vk::DeviceSize)std::ceil(m_BufferCreateInfo.size / atomSize) * atomSize;
	if (mapSize > m_BufferCreateInfo.size) {
		mapSize = VK_WHOLE_SIZE;
	}
	vk::MappedMemoryRange mr{ mem, offset, mapSize };
	device.flushMappedMemoryRanges(mr);
	*/

	device.unmapMemory(mem);

	spt->unlockPageMutex();
}

void ngv::VulkanBuffer::Upload(vk::CommandBuffer cb, VulkanBuffer& stagingBuffer, const void* value, vk::DeviceSize size)
{
#ifndef NDEBUG
	if (size == 0) {
		std::runtime_error("Tried to upload data with size 0 to buffer");
	}
	auto spt = m_pAllocation->getMemoryPage();
	if (spt == nullptr) {
		std::runtime_error("Tried to upload to buffer without allocation");
	}
#endif

	using buf = vk::BufferUsageFlagBits;
	using pfb = vk::MemoryPropertyFlagBits;
	stagingBuffer.UpdateLocal(value, size);

	vk::BufferCopy bc{ 0, 0, size};
	cb.copyBuffer(stagingBuffer.Buffer(), *m_Buffer, bc);
}

void ngv::VulkanBuffer::Barrier(vk::CommandBuffer cb, vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags dstStageMask, vk::DependencyFlags dependencyFlags, vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask, uint32 srcQueueFamilyIndex, uint32 dstQueueFamilyIndex) const
{
	vk::BufferMemoryBarrier bmb{ srcAccessMask, dstAccessMask, srcQueueFamilyIndex, dstQueueFamilyIndex, *m_Buffer, 0, VK_WHOLE_SIZE };
	cb.pipelineBarrier(srcStageMask, dstStageMask, dependencyFlags, nullptr, bmb, nullptr);
}

void ngv::VulkanBuffer::Flush()
{
	auto spt = m_pAllocation->getMemoryPage();

#ifndef NDEBUG
	if (spt == nullptr) {
		std::runtime_error("Tried to flush buffer with no allocation");
	}
#endif

	vk::DeviceSize atomSize = spt->vulkanDevice().physicalDeviceLimits().nonCoherentAtomSize;

	vk::DeviceMemory memory = spt->memory();
	vk::MappedMemoryRange mr{ memory, m_pAllocation->getOffset(), 
		(vk::DeviceSize)std::ceil(m_BufferCreateInfo.size / atomSize) * atomSize };
	spt->vulkanDevice().device().flushMappedMemoryRanges(mr);
}

void ngv::VulkanBuffer::Invalidate()
{
	auto spt = m_pAllocation->getMemoryPage();

#ifndef NDEBUG
	if (spt == nullptr) {
		std::runtime_error("Tried to flush buffer with no allocation");
	}
#endif

	vk::DeviceSize atomSize = spt->vulkanDevice().physicalDeviceLimits().nonCoherentAtomSize;

	vk::DeviceMemory memory = spt->memory();
	vk::MappedMemoryRange mr{ memory, m_pAllocation->getOffset(), 
		(vk::DeviceSize)std::ceil(m_BufferCreateInfo.size / atomSize) * atomSize };
	spt->vulkanDevice().device().invalidateMappedMemoryRanges(mr);
}

bool ngv::VulkanBuffer::HasAllocation()
{
	if (m_pAllocation->getMemoryPage() != nullptr) {
		return true;
	}
	return false;
}

bool ngv::VulkanBuffer::HasSameAllocation(const ngv::VulkanBuffer& buffer)
{
	if (m_pAllocation->getMemoryPage() != nullptr) {
		if (buffer.m_pAllocation->getMemoryPage() != nullptr) {
			if (m_pAllocation == buffer.m_pAllocation) { // equiv to m_pAllocation.get() == buffer->m_pAllocation.get()
				return true;
			}
		}
	}
	return false;
}

void ngv::VulkanBuffer::create(const ngv::VulkanDevice& device, const vk::BufferCreateInfo& info, bool hostBuffer)
{
	m_BufferCreateInfo = info;
	m_MemoryPropertyFlags =
		(!hostBuffer) ? vk::MemoryPropertyFlagBits::eDeviceLocal : (vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);
	m_Buffer = device.device().createBufferUnique(info);

	m_MemoryRequirements = device.device().getBufferMemoryRequirements(*m_Buffer);

	m_MemoryTypeIndex = ngv::findMemoryTypeIndex(device.physicalDeviceMemoryProperties(), m_MemoryRequirements.memoryTypeBits, m_MemoryPropertyFlags);

	m_Created = true;
}

ngv::VulkanBuffer::VulkanBuffer(const VulkanDevice& device, const vk::BufferCreateInfo& info, bool hostBuffer)
{
	create(device, info, hostBuffer);
}






// <============================ VULKAN SPARSE BUFFER =================================================>
/*
std::unique_ptr<ngv::VulkanSparseBuffer> ngv::VulkanSparseBuffer::make(VulkanDevice& device, const vk::ImageCreateInfo& info, vk::ImageViewType viewType, vk::ImageAspectFlags aspectMask)
{
	return std::unique_ptr<VulkanSparseBuffer>(new VulkanSparseBuffer(device, info, viewType, aspectMask));
}

// <======================== VULKAN SPARSE BUFFER ====================================>
void ngv::VulkanSparseBuffer::create(VulkanDevice& device, const vk::BufferCreateInfo& info)
{
	m_Buffer = device.device().createBufferUnique(info);
	
	m_MemoryRequirements = device.device().getBufferMemoryRequirements(*m_Buffer);

	if (m_MemoryRequirements.size > device.physicalDeviceProperties().limits.sparseAddressSpaceSize) {
		std::runtime_error("Requested sparse buffer size exceeds supported sparse address space size");
	}

	

}

ngv::VulkanSparseBuffer::VulkanSparseBuffer(VulkanDevice& device, const vk::BufferCreateInfo& info)
{
	create(device, info);
}
*/








// <=============== VULKAN IMAGE ========================>

std::unique_ptr<ngv::VulkanImage> ngv::VulkanImage::Make(const VulkanDevice& device, const vk::ImageCreateInfo& info, bool hostImage)
{
	return std::unique_ptr<VulkanImage>(new VulkanImage(device, info, hostImage));
}

void ngv::VulkanImage::CreateImageView(vk::ImageViewType viewType, vk::ImageAspectFlags aspectMask)
{
	auto spt = m_pAllocation->getMemoryPage();
#ifndef NDEBUG
	if (spt == nullptr) {
		std::runtime_error("Tried to create image view for image without allocation");
	}
#endif
	bool hostImage = true;
	if (m_MemoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal) {
		hostImage = false;
	}

	if (!hostImage) {
		vk::ImageViewCreateInfo viewInfo{};
		viewInfo.image = *m_Image;
		viewInfo.viewType = viewType;
		viewInfo.format = m_ImageCreateInfo.format;
		viewInfo.components = { vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA };
		viewInfo.subresourceRange = vk::ImageSubresourceRange{ aspectMask, 0, m_ImageCreateInfo.mipLevels, 0, m_ImageCreateInfo.arrayLayers };
		m_ImageView = spt->vulkanDevice().device().createImageViewUnique(viewInfo);
	}

}

vk::Image ngv::VulkanImage::Image() const
{
	return *m_Image;
}

vk::ImageView ngv::VulkanImage::ImageView() const
{
	return *m_ImageView;
}

vk::Format ngv::VulkanImage::Format() const
{
	return m_ImageCreateInfo.format;
}

vk::Extent3D ngv::VulkanImage::Extent() const
{
	return m_ImageCreateInfo.extent;
}

void ngv::VulkanImage::Clear(vk::CommandBuffer cb, const std::array<float, 4> color)
{
	SetLayout(cb, vk::ImageLayout::eTransferDstOptimal);
	vk::ClearColorValue ccv(color);
	vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
	cb.clearColorImage(*m_Image, vk::ImageLayout::eTransferDstOptimal, ccv, range);
}

void ngv::VulkanImage::Copy(vk::CommandBuffer cb, ngv::VulkanImage& srcImage)
{
	srcImage.SetLayout(cb, vk::ImageLayout::eTransferSrcOptimal);
	SetLayout(cb, vk::ImageLayout::eTransferDstOptimal);
	for (uint32 mipLevel = 0; mipLevel != m_ImageCreateInfo.mipLevels; ++mipLevel) {
		vk::ImageCopy region{};
		region.srcSubresource = { vk::ImageAspectFlagBits::eColor, mipLevel, 0,1 };
		region.dstSubresource = { vk::ImageAspectFlagBits::eColor, mipLevel, 0, 1 };
		region.extent = m_ImageCreateInfo.extent;
		cb.copyImage(srcImage.Image(), vk::ImageLayout::eTransferSrcOptimal, *m_Image, vk::ImageLayout::eTransferDstOptimal, region);
	}
}

void ngv::VulkanImage::Copy(vk::CommandBuffer cb, vk::Buffer buffer, uint32 mipLevel, uint32 arrayLayer, uint32 width, uint32 height, uint32 depth, uint32 offset)
{
	SetLayout(cb, vk::ImageLayout::eTransferDstOptimal);
	vk::BufferImageCopy region{};
	region.bufferOffset = offset;
	vk::Extent3D extent;
	extent.width = width;
	extent.height = height;
	extent.depth = depth;
	region.imageSubresource = { vk::ImageAspectFlagBits::eColor, mipLevel, arrayLayer, 1 };
	region.imageExtent = extent;
	cb.copyBufferToImage(buffer, *m_Image, vk::ImageLayout::eTransferDstOptimal, region);
}

void ngv::VulkanImage::Upload(vk::CommandBuffer cb, VulkanBuffer& stagingBuffer, const void* value, vk::DeviceSize size)
{
	stagingBuffer.UpdateLocal(value, size);

	auto bp = ngv::getBlockParams(m_ImageCreateInfo.format);
	vk::Buffer buf = stagingBuffer.Buffer();
	uint32 offset = 0;
	for (uint32 mipLevel = 0; mipLevel != m_ImageCreateInfo.mipLevels; ++mipLevel) {
		auto width = mipScale(m_ImageCreateInfo.extent.width, mipLevel);
		auto height = mipScale(m_ImageCreateInfo.extent.height, mipLevel);
		auto depth = mipScale(m_ImageCreateInfo.extent.depth, mipLevel);
		for (uint32 face = 0; face != m_ImageCreateInfo.arrayLayers; ++face) {
			Copy(cb, buf, mipLevel, face, width, height, depth, offset);
			offset += ((bp.bytesPerBlock + 3) & ~3) * (width * height);
		}
	}
	SetLayout(cb, vk::ImageLayout::eShaderReadOnlyOptimal);
}

const vk::ImageCreateInfo ngv::VulkanImage::ImageCreateInfo() const
{
	return m_ImageCreateInfo;
}

const vk::MemoryPropertyFlags ngv::VulkanImage::MemoryPropertyFlags() const
{
	return m_MemoryPropertyFlags;
}

void ngv::VulkanImage::SetLayout(vk::CommandBuffer cb, vk::ImageLayout newLayout, vk::ImageAspectFlags aspectMask)
{
	if (newLayout == m_CurrentLayout) {
		return;
	}
	vk::ImageLayout oldLayout = m_CurrentLayout;
	m_CurrentLayout = newLayout;

	vk::ImageMemoryBarrier imageMemoryBarriers = {};
	imageMemoryBarriers.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarriers.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarriers.oldLayout = oldLayout;
	imageMemoryBarriers.newLayout = newLayout;
	imageMemoryBarriers.image = *m_Image;
	imageMemoryBarriers.subresourceRange = { aspectMask, 0, m_ImageCreateInfo.mipLevels, 0, m_ImageCreateInfo.arrayLayers };

	vk::PipelineStageFlags srcStageMask{ vk::PipelineStageFlagBits::eTopOfPipe };
	vk::PipelineStageFlags dstStageMask{ vk::PipelineStageFlagBits::eTopOfPipe };
	vk::DependencyFlags dependencyFlags{};
	vk::AccessFlags srcMask{};
	vk::AccessFlags dstMask{};

	typedef vk::ImageLayout il;
	typedef vk::AccessFlagBits afb;

	switch (oldLayout) {
	case il::eUndefined: break;
	case il::eGeneral: srcMask = afb::eTransferWrite; break;
	case il::eColorAttachmentOptimal: srcMask = afb::eColorAttachmentWrite; break;
	case il::eDepthStencilAttachmentOptimal: srcMask = afb::eDepthStencilAttachmentWrite; break;
	case il::eDepthStencilReadOnlyOptimal: srcMask = afb::eDepthStencilAttachmentRead; break;
	case il::eShaderReadOnlyOptimal: srcMask = afb::eShaderRead; break;
	case il::eTransferSrcOptimal: srcMask = afb::eTransferRead; break;
	case il::eTransferDstOptimal: srcMask = afb::eTransferWrite; break;
	case il::ePreinitialized: srcMask = afb::eTransferWrite | afb::eHostWrite; break;
	case il::ePresentSrcKHR: srcMask = afb::eMemoryRead; break;
	}

	switch (newLayout) {
	case il::eUndefined: break;
	case il::eGeneral: dstMask = afb::eTransferWrite; break;
	case il::eColorAttachmentOptimal: dstMask = afb::eColorAttachmentWrite; break;
	case il::eDepthStencilAttachmentOptimal: dstMask = afb::eDepthStencilAttachmentWrite; break;
	case il::eDepthStencilReadOnlyOptimal: dstMask = afb::eDepthStencilAttachmentRead; break;
	case il::eShaderReadOnlyOptimal: dstMask = afb::eShaderRead; break;
	case il::eTransferSrcOptimal: dstMask = afb::eTransferRead; break;
	case il::eTransferDstOptimal: dstMask = afb::eTransferWrite; break;
	case il::ePreinitialized: dstMask = afb::eTransferWrite; break;
	case il::ePresentSrcKHR: dstMask = afb::eMemoryRead; break;
	}

	imageMemoryBarriers.srcAccessMask = srcMask;
	imageMemoryBarriers.dstAccessMask = dstMask;
	auto memoryBarriers = nullptr;
	auto bufferMemoryBarriers = nullptr;
	cb.pipelineBarrier(srcStageMask, dstStageMask, dependencyFlags, memoryBarriers, bufferMemoryBarriers, imageMemoryBarriers);

}

void ngv::VulkanImage::SetCurrentLayout(vk::ImageLayout oldLayout)
{
	m_CurrentLayout = oldLayout;
}

vk::ImageLayout ngv::VulkanImage::GetImageLayout()
{
	return m_CurrentLayout;
}

void ngv::VulkanImage::SetImageLayout(vk::ImageLayout imageLayout)
{
	m_CurrentLayout = imageLayout;
}

bool ngv::VulkanImage::HasAllocation()
{
	if (m_pAllocation->getMemoryPage() != nullptr) {
		return true;
	}
	return false;
}

bool ngv::VulkanImage::HasSameAllocation(const VulkanImage& image)
{
	if (m_pAllocation->getMemoryPage()) {
		if (image.m_pAllocation->getMemoryPage()) {
			if (m_pAllocation == image.m_pAllocation) {
				return true;
			}
		}
	}
	return false;
}

void ngv::VulkanImage::create(const ngv::VulkanDevice& device, const vk::ImageCreateInfo& info, bool hostImage)
{
	m_ImageCreateInfo = info;
	m_MemoryPropertyFlags =
		(!hostImage) ? vk::MemoryPropertyFlagBits::eDeviceLocal : (vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	m_CurrentLayout = info.initialLayout;

	m_Image = device.device().createImageUnique(info);

	m_MemoryRequirements = device.device().getImageMemoryRequirements(*m_Image);

	m_MemoryTypeIndex = ngv::findMemoryTypeIndex(device.physicalDeviceMemoryProperties(), m_MemoryRequirements.memoryTypeBits, m_MemoryPropertyFlags);

	m_Created = true;
}

ngv::VulkanImage::VulkanImage(const VulkanDevice& device, const vk::ImageCreateInfo& info, bool hostImage)
{
	create(device, info, hostImage);
}









// <=============================== VERTEX BUFFER ====================================>
std::unique_ptr<ngv::VulkanVertexBuffer> ngv::VulkanVertexBuffer::Make(const VulkanDevice& device, vk::DeviceSize size, bool hostBuffer)
{
	return std::unique_ptr<VulkanVertexBuffer>(new VulkanVertexBuffer(device, size, hostBuffer));
}

ngv::VulkanVertexBuffer::VulkanVertexBuffer(const VulkanDevice& device, vk::DeviceSize size, bool hostBuffer)
{
	vk::BufferCreateInfo ci{};
	ci.size = size;
	ci.usage = vk::BufferUsageFlagBits::eVertexBuffer | 
		vk::BufferUsageFlagBits::eTransferDst | 
		vk::BufferUsageFlagBits::eTransferSrc;
	ci.sharingMode = vk::SharingMode::eExclusive;
	create(device, ci, hostBuffer);
}










// <====================================== INDEX BUFFER ======================================>
std::unique_ptr<ngv::VulkanIndexBuffer> ngv::VulkanIndexBuffer::Make(const VulkanDevice& device, vk::DeviceSize size, bool hostBuffer)
{
	return std::unique_ptr<VulkanIndexBuffer>(new VulkanIndexBuffer(device, size, hostBuffer));
}

ngv::VulkanIndexBuffer::VulkanIndexBuffer(const VulkanDevice& device, vk::DeviceSize size, bool hostBuffer)
{
	vk::BufferCreateInfo ci{};
	ci.size = size;
	ci.usage = vk::BufferUsageFlagBits::eIndexBuffer | 
		vk::BufferUsageFlagBits::eTransferDst | 
		vk::BufferUsageFlagBits::eTransferSrc;
	ci.sharingMode = vk::SharingMode::eExclusive;
	create(device, ci, hostBuffer);
}









// <===================================== UNIFORM BUFFER ===========================================>
std::unique_ptr<ngv::VulkanUniformBuffer> ngv::VulkanUniformBuffer::Make(const VulkanDevice& device, vk::DeviceSize size, bool hostBuffer)
{
	return std::unique_ptr<VulkanUniformBuffer>(new VulkanUniformBuffer(device, size, hostBuffer));
}

ngv::VulkanUniformBuffer::VulkanUniformBuffer(const VulkanDevice& device, vk::DeviceSize size, bool hostBuffer)
{
	vk::BufferCreateInfo ci{};
	ci.size = size;
	ci.usage = vk::BufferUsageFlagBits::eUniformBuffer |
		vk::BufferUsageFlagBits::eTransferDst |
		vk::BufferUsageFlagBits::eTransferSrc;
	ci.sharingMode = vk::SharingMode::eExclusive;
	create(device, ci, hostBuffer);
}










// <===================================== TEXTURE 2D =================================================>
std::unique_ptr<ngv::VulkanTexture2D> ngv::VulkanTexture2D::Make(const VulkanDevice& device, uint32 width, uint32 height, uint32 mipLevels, vk::Format format, vk::SampleCountFlagBits sampleFlags, bool hostImage)
{
	return std::unique_ptr<VulkanTexture2D>(new VulkanTexture2D(device, width, height, mipLevels, format, sampleFlags, hostImage));
}

void ngv::VulkanTexture2D::CreateImageView()
{
	VulkanImage::CreateImageView(vk::ImageViewType::e2D, vk::ImageAspectFlagBits::eColor);
}

ngv::VulkanTexture2D::VulkanTexture2D(const VulkanDevice& device, uint32 width, uint32 height, uint32 mipLevels, vk::Format format, vk::SampleCountFlagBits sampleFlags, bool hostImage)
{
	vk::ImageCreateInfo ci{};
	ci.flags = {};
	ci.imageType = vk::ImageType::e2D;
	ci.format = format;
	ci.extent = vk::Extent3D{ width, height, 1U };
	ci.mipLevels = mipLevels;
	ci.arrayLayers = 1;
	ci.samples = sampleFlags;
	ci.tiling = hostImage ? vk::ImageTiling::eLinear : vk::ImageTiling::eOptimal;
	ci.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst;
	ci.sharingMode = vk::SharingMode::eExclusive;
	ci.queueFamilyIndexCount = 0;
	ci.pQueueFamilyIndices = nullptr;
	ci.initialLayout = hostImage ? vk::ImageLayout::ePreinitialized : vk::ImageLayout::eUndefined;
	create(device, ci, hostImage);
	//LEFT HERE
}











// <================================= TEXTURE CUBE =======================================================>
std::unique_ptr<ngv::VulkanTextureCube> ngv::VulkanTextureCube::Make(const VulkanDevice& device, uint32 width, uint32 height, vk::Format format, uint32 mipLevels, vk::SampleCountFlagBits sampleFlags, bool hostImage)
{
	return std::unique_ptr<VulkanTextureCube>(new VulkanTextureCube(device, width, height, format, mipLevels, sampleFlags, hostImage));
}

void ngv::VulkanTextureCube::CreateImageView()
{
	VulkanImage::CreateImageView(vk::ImageViewType::eCube, vk::ImageAspectFlagBits::eColor);
}

ngv::VulkanTextureCube::VulkanTextureCube(const VulkanDevice& device, uint32 width, uint32 height, vk::Format format, uint32 mipLevels, vk::SampleCountFlagBits sampleFlags, bool hostImage)
{
	vk::ImageCreateInfo ci{};
	ci.flags = { vk::ImageCreateFlagBits::eCubeCompatible };
	ci.imageType = vk::ImageType::e2D;
	ci.format = format;
	ci.extent = vk::Extent3D{ width, height, 1U };
	ci.mipLevels = mipLevels;
	ci.arrayLayers = 6;
	ci.samples = sampleFlags;
	ci.tiling = hostImage ? vk::ImageTiling::eLinear : vk::ImageTiling::eOptimal;
	ci.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst;
	ci.sharingMode = vk::SharingMode::eExclusive;
	ci.queueFamilyIndexCount = 0;
	ci.pQueueFamilyIndices = nullptr;
	ci.initialLayout = vk::ImageLayout::ePreinitialized;
	create(device, ci, hostImage);
}












// <===================================== DEPTH STENCIL ================================================>
std::unique_ptr<ngv::VulkanDepthStencilImage> ngv::VulkanDepthStencilImage::Make(const VulkanDevice& device, uint32 width,
	uint32 height, vk::Format format, vk::SampleCountFlagBits sampleFlags)
{
	return std::unique_ptr<VulkanDepthStencilImage>(new VulkanDepthStencilImage(device, width, height, format, sampleFlags));
}

void ngv::VulkanDepthStencilImage::CreateImageView()
{
	typedef vk::ImageAspectFlagBits iafb;
	VulkanImage::CreateImageView(vk::ImageViewType::e2D, iafb::eDepth);
}

ngv::VulkanDepthStencilImage::VulkanDepthStencilImage(const VulkanDevice& device, uint32 width, uint32 height, vk::Format format, vk::SampleCountFlagBits sampleFlags)
{
	vk::ImageCreateInfo ci{};
	ci.flags = {};
	ci.imageType = vk::ImageType::e2D;
	ci.format = format;
	ci.extent = vk::Extent3D{ width, height, 1U };
	ci.mipLevels = 1;
	ci.arrayLayers = 1;
	ci.samples = sampleFlags;
	ci.tiling = vk::ImageTiling::eOptimal;
	ci.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled;
	ci.sharingMode = vk::SharingMode::eExclusive;
	ci.queueFamilyIndexCount = 0;
	ci.pQueueFamilyIndices = nullptr;
	ci.initialLayout = vk::ImageLayout::eUndefined;
	create(device, ci, false);
}










// <========================================= COLOR ATTACHMENT ========================================>
std::unique_ptr<ngv::VulkanColorAttachmentImage> ngv::VulkanColorAttachmentImage::Make(const VulkanDevice& device, uint32 width, uint32 height, vk::Format format, vk::SampleCountFlagBits sampleFlags)
{
	return std::unique_ptr<VulkanColorAttachmentImage>(new VulkanColorAttachmentImage(device, width, height, format, sampleFlags));
}

void ngv::VulkanColorAttachmentImage::CreateImageView()
{
	typedef vk::ImageAspectFlagBits iafb;
	VulkanImage::CreateImageView(vk::ImageViewType::e2D, iafb::eColor);
}

ngv::VulkanColorAttachmentImage::VulkanColorAttachmentImage(const VulkanDevice& device, uint32 width, uint32 height, vk::Format format, vk::SampleCountFlagBits sampleFlags)
{
	vk::ImageCreateInfo ci{};
	ci.flags = {};
	ci.imageType = vk::ImageType::e2D;
	ci.format = format;
	ci.extent = vk::Extent3D{ width, height, 1U };
	ci.mipLevels = 1;
	ci.arrayLayers = 1;
	ci.samples = sampleFlags;
	ci.tiling = vk::ImageTiling::eOptimal;
	ci.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled;
	ci.sharingMode = vk::SharingMode::eExclusive;
	ci.queueFamilyIndexCount = 0;
	ci.pQueueFamilyIndices = nullptr;
	ci.initialLayout = vk::ImageLayout::eUndefined;
	create(device, ci, false);
}










// <==================================== SAMPLER MAKER ====================================>
ngv::VulkanSamplerMaker::VulkanSamplerMaker()
{
	m_Info.magFilter = vk::Filter::eNearest;
	m_Info.minFilter = vk::Filter::eNearest;
	m_Info.mipmapMode = vk::SamplerMipmapMode::eNearest;
	m_Info.addressModeU = vk::SamplerAddressMode::eRepeat;
	m_Info.addressModeV = vk::SamplerAddressMode::eRepeat;
	m_Info.addressModeW = vk::SamplerAddressMode::eRepeat;
	m_Info.mipLodBias = 0.0f;
	m_Info.anisotropyEnable = 0;
	m_Info.maxAnisotropy = 0.0f;
	m_Info.compareEnable = 0;
	m_Info.compareOp = vk::CompareOp::eNever;
	m_Info.minLod = 0;
	m_Info.maxLod = 0;
	m_Info.borderColor = vk::BorderColor{};
	m_Info.unnormalizedCoordinates = 0;
}

ngv::VulkanSamplerMaker& ngv::VulkanSamplerMaker::flags(vk::SamplerCreateFlags value)
{
	m_Info.flags = value;
	return *this;
}

ngv::VulkanSamplerMaker& ngv::VulkanSamplerMaker::magFilter(vk::Filter filter)
{
	m_Info.magFilter = filter;
	return *this;
}

ngv::VulkanSamplerMaker& ngv::VulkanSamplerMaker::minFilter(vk::Filter filter)
{
	m_Info.minFilter = filter;
	return *this;
}

ngv::VulkanSamplerMaker& ngv::VulkanSamplerMaker::mipmapMode(vk::SamplerMipmapMode value)
{
	m_Info.mipmapMode = value;
	return *this;
}

ngv::VulkanSamplerMaker& ngv::VulkanSamplerMaker::addressModeU(vk::SamplerAddressMode value)
{
	m_Info.addressModeU = value;
	return *this;
}

ngv::VulkanSamplerMaker& ngv::VulkanSamplerMaker::addressModeV(vk::SamplerAddressMode value)
{
	m_Info.addressModeV = value;
	return *this;
}

ngv::VulkanSamplerMaker& ngv::VulkanSamplerMaker::addressModeW(vk::SamplerAddressMode value)
{
	m_Info.addressModeW = value;
	return *this;

}

ngv::VulkanSamplerMaker& ngv::VulkanSamplerMaker::mipLodBias(float value)
{
	m_Info.mipLodBias = value;
	return *this;
}

ngv::VulkanSamplerMaker& ngv::VulkanSamplerMaker::anisotropyEnable(vk::Bool32 value)
{
	m_Info.anisotropyEnable = value;
	return *this;
}

ngv::VulkanSamplerMaker& ngv::VulkanSamplerMaker::maxAnisotropy(float value)
{
	m_Info.maxAnisotropy = value;
	return *this;
}

ngv::VulkanSamplerMaker& ngv::VulkanSamplerMaker::compareEnable(vk::Bool32 value)
{
	m_Info.compareEnable = value;
	return *this;
}

ngv::VulkanSamplerMaker& ngv::VulkanSamplerMaker::compareOp(vk::CompareOp value)
{
	m_Info.compareOp = value;
	return *this;
}

ngv::VulkanSamplerMaker& ngv::VulkanSamplerMaker::minLod(float value)
{
	m_Info.minLod = value;
	return *this;
}

ngv::VulkanSamplerMaker& ngv::VulkanSamplerMaker::maxLod(float value)
{
	m_Info.maxLod = value;
	return *this;
}

ngv::VulkanSamplerMaker& ngv::VulkanSamplerMaker::borderColor(vk::BorderColor value)
{
	m_Info.borderColor = value;
	return *this;
}

ngv::VulkanSamplerMaker& ngv::VulkanSamplerMaker::unnormalizedCoordinates(vk::Bool32 value)
{
	m_Info.unnormalizedCoordinates = value;
	return *this;
}


vk::Sampler ngv::VulkanSamplerMaker::create(vk::Device device) const
{
	return device.createSampler(m_Info);
}

vk::UniqueSampler ngv::VulkanSamplerMaker::createUnique(vk::Device device) const
{
	return device.createSamplerUnique(m_Info);
}
