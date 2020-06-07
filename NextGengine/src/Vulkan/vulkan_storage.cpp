#include "vulkan_storage.h"

#include "vulkan_allocator.h"

#include "vulkan_utility.h"

#include "vulkan_device.h"





// <==================== VULKAN BUFFER ==========================>

std::unique_ptr<ngv::VulkanBuffer> ngv::VulkanBuffer::make(VulkanDevice& device, const vk::BufferCreateInfo& info, bool hostBuffer)
{
	return std::unique_ptr<VulkanBuffer>(new VulkanBuffer(device, info, hostBuffer));
}

vk::Buffer ngv::VulkanBuffer::buffer() const
{
	return *m_Buffer;
}

const vk::BufferCreateInfo ngv::VulkanBuffer::getBufferCreateInfo() const
{
	return m_BufferCreateInfo;
}

const vk::MemoryPropertyFlags ngv::VulkanBuffer::getMemoryPropertyFlags() const
{
	return m_MemoryPropertyFlags;
}

void* ngv::VulkanBuffer::map()
{
	auto spt = m_pMemoryPage.lock();
#ifndef NDEBUG
	if (spt == nullptr)	return false;
	if (m_pAllocation == nullptr) return false;
#endif

	spt->device().mapMemory(spt->memory(), m_pAllocation->getOffset(), m_BufferCreateInfo.size);
}

void ngv::VulkanBuffer::unmap()
{
	auto spt = m_pMemoryPage.lock();
	return spt->device().unmapMemory(spt->memory());
}

bool ngv::VulkanBuffer::updateLocal(const void* value, vk::DeviceSize size) const
{
	auto spt = m_pMemoryPage.lock();

#ifndef NDEBUG
	if (size == 0) return false;
	if (spt == nullptr)	return false;
	if (m_pAllocation == nullptr) return false;
#endif

	std::lock_guard<std::mutex> lock(spt->pageMutex);
	vk::Device ldevice = spt->device();
	vk::DeviceMemory mem = spt->memory();
	vk::DeviceSize offset = m_pAllocation->getOffset();

	void* ptr = ldevice.mapMemory(mem, offset, size);
	memcpy(ptr, value, (size_t)size);
	vk::MappedMemoryRange mr{ mem, offset, size };
	ldevice.flushMappedMemoryRanges(mr);
	ldevice.unmapMemory(mem);
	return true;
}

bool ngv::VulkanBuffer::upload(vk::CommandBuffer cb, std::shared_ptr<VulkanBuffer> stagingBuffer, const void* value, vk::DeviceSize size)
{
#ifndef NDEBUG
	if (size == 0) return false;
	auto spt = m_pMemoryPage.lock();
	if (spt == nullptr)	return false;
	if (m_pAllocation == nullptr) return false;
#endif

	using buf = vk::BufferUsageFlagBits;
	using pfb = vk::MemoryPropertyFlagBits;
	stagingBuffer->updateLocal(value, size);

	vk::BufferCopy bc{ 0, 0, size};
	cb.copyBuffer(stagingBuffer->buffer(), *m_Buffer, bc);
	return true;
}

void ngv::VulkanBuffer::barrier(vk::CommandBuffer cb, vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags dstStageMask, vk::DependencyFlags dependencyFlags, vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask, uint32 srcQueueFamilyIndex, uint32 dstQueueFamilyIndex) const
{
	vk::BufferMemoryBarrier bmb{ srcAccessMask, dstAccessMask, srcQueueFamilyIndex, dstQueueFamilyIndex, *m_Buffer, 0, VK_WHOLE_SIZE };
	cb.pipelineBarrier(srcStageMask, dstStageMask, dependencyFlags, nullptr, bmb, nullptr);
}

bool ngv::VulkanBuffer::flush(const VulkanDevice& device)
{
	auto spt = m_pMemoryPage.lock();
	if (!spt) {
		return false;
	}
	vk::DeviceMemory memory = spt->memory();
	vk::MappedMemoryRange mr{ memory, m_pAllocation->getOffset(), m_BufferCreateInfo.size };
	device.device().flushMappedMemoryRanges(mr);
	return true;
}

bool ngv::VulkanBuffer::invalidate(const VulkanDevice& device)
{
	auto spt = m_pMemoryPage.lock();
	if (!spt) {
		return false;
	}
	vk::DeviceMemory memory = spt->memory();
	vk::MappedMemoryRange mr{ memory, m_pAllocation->getOffset(), m_BufferCreateInfo.size };
	device.device().invalidateMappedMemoryRanges(mr);
	return true;
}

bool ngv::VulkanBuffer::hasAllocation()
{
	if (auto spt = m_pMemoryPage.lock()) {
		return true;
	}
	return false;
}

bool ngv::VulkanBuffer::hasSameAllocation(const ngv::VulkanBuffer& buffer)
{
	if (auto spt1 = m_pMemoryPage.lock()) {
		if (auto spt2 = buffer.m_pMemoryPage.lock()) {
			if (m_pAllocation == buffer.m_pAllocation) { // equiv to m_pAllocation.get() == buffer->m_pAllocation.get()
				return true;
			}
		}
	}
	return false;
}

void ngv::VulkanBuffer::create(ngv::VulkanDevice& device, const vk::BufferCreateInfo& info, bool hostBuffer)
{
	m_BufferCreateInfo = info;
	m_MemoryPropertyFlags =
		(hostBuffer) ? vk::MemoryPropertyFlagBits::eDeviceLocal : vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible;
	m_Buffer = device.device().createBufferUnique(info);

	m_MemoryRequirements = device.device().getBufferMemoryRequirements(*m_Buffer);

	m_MemoryTypeIndex = ngv::findMemoryTypeIndex(device.physicalDeviceMemoryProperties(), m_MemoryRequirements.memoryTypeBits, m_MemoryPropertyFlags);

	m_Created = true;
}

ngv::VulkanBuffer::VulkanBuffer(VulkanDevice& device, const vk::BufferCreateInfo& info, bool hostBuffer)
{
	create(device, info, hostBuffer);
}










// <=============== VULKAN IMAGE ========================>

std::unique_ptr<ngv::VulkanImage> ngv::VulkanImage::make(VulkanDevice& device, const vk::ImageCreateInfo& info, vk::ImageViewType viewType, vk::ImageAspectFlags aspectMask, bool hostImage)
{
	return std::unique_ptr<VulkanImage>(new VulkanImage(device, info, viewType, aspectMask, hostImage));
}

vk::Image ngv::VulkanImage::image() const
{
	return *m_Image;
}

vk::ImageView ngv::VulkanImage::imageView() const
{
	return *m_ImageView;
}

vk::Format ngv::VulkanImage::format() const
{
	return m_ImageCreateInfo.format;
}

vk::Extent3D ngv::VulkanImage::extent() const
{
	return m_ImageCreateInfo.extent;
}

void ngv::VulkanImage::clear(vk::CommandBuffer cb, const std::array<float, 4> color)
{
	setLayout(cb, vk::ImageLayout::eTransferDstOptimal);
	vk::ClearColorValue ccv(color);
	vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
	cb.clearColorImage(*m_Image, vk::ImageLayout::eTransferDstOptimal, ccv, range);
}

void ngv::VulkanImage::copy(vk::CommandBuffer cb, ngv::VulkanImage& srcImage)
{
	srcImage.setLayout(cb, vk::ImageLayout::eTransferSrcOptimal);
	setLayout(cb, vk::ImageLayout::eTransferDstOptimal);
	for (uint32 mipLevel = 0; mipLevel != m_ImageCreateInfo.mipLevels; ++mipLevel) {
		vk::ImageCopy region{};
		region.srcSubresource = { vk::ImageAspectFlagBits::eColor, mipLevel, 0,1 };
		region.dstSubresource = { vk::ImageAspectFlagBits::eColor, mipLevel, 0, 1 };
		region.extent = m_ImageCreateInfo.extent;
		cb.copyImage(srcImage.image(), vk::ImageLayout::eTransferSrcOptimal, *m_Image, vk::ImageLayout::eTransferDstOptimal, region);
	}
}

void ngv::VulkanImage::copy(vk::CommandBuffer cb, vk::Buffer buffer, uint32 mipLevel, uint32 arrayLayer, uint32 width, uint32 height, uint32 depth, uint32 offset)
{
	setLayout(cb, vk::ImageLayout::eTransferDstOptimal);
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

void ngv::VulkanImage::upload(vk::CommandBuffer cb, std::shared_ptr<VulkanBuffer> stagingBuffer, const void* value, vk::DeviceSize size)
{
	stagingBuffer->updateLocal(value, size);

	auto bp = ngv::getBlockParams(m_ImageCreateInfo.format);
	vk::Buffer buf = stagingBuffer->buffer();
	uint32 offset = 0;
	for (uint32 mipLevel = 0; mipLevel != m_ImageCreateInfo.mipLevels; ++mipLevel) {
		auto width = mipScale(m_ImageCreateInfo.extent.width, mipLevel);
		auto height = mipScale(m_ImageCreateInfo.extent.height, mipLevel);
		auto depth = mipScale(m_ImageCreateInfo.extent.depth, mipLevel);
		for (uint32 face = 0; face != m_ImageCreateInfo.arrayLayers; ++face) {
			copy(cb, buf, mipLevel, face, width, height, depth, offset);
			offset += ((bp.bytesPerBlock + 3) & ~3) * (width * height);
		}
	}
	setLayout(cb, vk::ImageLayout::eShaderReadOnlyOptimal);
}

const vk::ImageCreateInfo ngv::VulkanImage::imageCreateInfo() const
{
	return m_ImageCreateInfo;
}

const vk::MemoryPropertyFlags ngv::VulkanImage::memoryPropertyFlags() const
{
	return m_MemoryPropertyFlags;
}

void ngv::VulkanImage::setLayout(vk::CommandBuffer cb, vk::ImageLayout newLayout, vk::ImageAspectFlags aspectMask)
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

void ngv::VulkanImage::setCurrentLayout(vk::ImageLayout oldLayout)
{
	m_CurrentLayout = oldLayout;
}

bool ngv::VulkanImage::hasAllocation()
{
	if (auto spt = m_pMemoryPage.lock()) {
		return true;
	}
	return false;
}

bool ngv::VulkanImage::hasSameAllocation(const VulkanImage& image)
{
	if (auto spt1 = m_pMemoryPage.lock()) {
		if (auto spt2 = image.m_pMemoryPage.lock()) {
			if (m_pAllocation == image.m_pAllocation) {
				return true;
			}
		}
	}
	return false;
}

void ngv::VulkanImage::create(ngv::VulkanDevice& device, const vk::ImageCreateInfo& info, vk::ImageViewType viewType, vk::ImageAspectFlags aspectMask, bool hostImage)
{
	m_ImageCreateInfo = info;
	m_MemoryPropertyFlags =
		(hostImage) ? vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible : vk::MemoryPropertyFlagBits::eDeviceLocal;
	m_CurrentLayout = info.initialLayout;

	m_Image = device.device().createImageUnique(info);

	if (!hostImage) {
		vk::ImageViewCreateInfo viewInfo{};
		viewInfo.image = *m_Image;
		viewInfo.viewType = viewType;
		viewInfo.format = info.format;
		viewInfo.components = { vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA };
		viewInfo.subresourceRange = vk::ImageSubresourceRange{ aspectMask, 0, info.mipLevels, 0, info.arrayLayers };
		m_ImageView = device.device().createImageViewUnique(viewInfo);
	}

	m_MemoryRequirements = device.device().getImageMemoryRequirements(*m_Image);


	m_MemoryTypeIndex = ngv::findMemoryTypeIndex(device.physicalDeviceMemoryProperties(), m_MemoryRequirements.memoryTypeBits, m_MemoryPropertyFlags);

	m_Created = true;
}

ngv::VulkanImage::VulkanImage(VulkanDevice& device, const vk::ImageCreateInfo& info, vk::ImageViewType viewType, vk::ImageAspectFlags aspectMask, bool hostImage)
{
	create(device, info, viewType, aspectMask, hostImage);
}









// <=============================== VERTEX BUFFER ====================================>
std::unique_ptr<ngv::VertexBuffer> ngv::VertexBuffer::make(VulkanDevice& device, vk::DeviceSize size, bool hostBuffer)
{
	return std::unique_ptr<VertexBuffer>(new VertexBuffer(device, size, hostBuffer));
}

ngv::VertexBuffer::VertexBuffer(VulkanDevice& device, vk::DeviceSize size, bool hostBuffer)
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
std::unique_ptr<ngv::IndexBuffer> ngv::IndexBuffer::make(VulkanDevice& device, vk::DeviceSize size, bool hostBuffer)
{
	return std::unique_ptr<IndexBuffer>(new IndexBuffer(device, size, hostBuffer));
}

ngv::IndexBuffer::IndexBuffer(VulkanDevice& device, vk::DeviceSize size, bool hostBuffer)
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
std::unique_ptr<ngv::UniformBuffer> ngv::UniformBuffer::make(VulkanDevice& device, vk::DeviceSize size, bool hostBuffer)
{
	return std::unique_ptr<UniformBuffer>(new UniformBuffer(device, size, hostBuffer));
}

ngv::UniformBuffer::UniformBuffer(VulkanDevice& device, vk::DeviceSize size, bool hostBuffer)
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
std::unique_ptr<ngv::Texture2D> ngv::Texture2D::make(VulkanDevice& device, uint32 width, uint32 height, uint32 mipLevels, vk::Format format, vk::SampleCountFlagBits sampleFlags,bool hostImage = false)
{
	return std::unique_ptr<Texture2D>(new Texture2D(device, width, height, mipLevels, format, hostImage));
}

ngv::Texture2D::Texture2D(VulkanDevice& device, uint32 width, uint32 height, uint32 mipLevels, vk::Format format, vk::SampleCountFlagBits sampleFlags, bool hostImage)
{
	vk::ImageCreateInfo ci{};
	ci.flags = {};
	ci.imageType = vk::ImageType::e2D;
	ci.format = format;
	ci.extent = vk::Extent3D{ width, height, 1U };
	ci.mipLevels = mipLevels;
	ci.arrayLayers = 1;
	ci.samples = sampleFlags;

	//LEFT HERE
}
