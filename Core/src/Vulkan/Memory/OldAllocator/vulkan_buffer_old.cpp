#include "vulkan_buffer.h"
#include "vulkan_buffer_region_allocator.h"
#include "../../Math/hash_functions.h"

void ng::vulkan::VulkanBuffer::update(VkDeviceSize newOffset, VkDeviceSize newSize, void* newData, VkDeviceSize newDataSize)
{
	m_Offset = newOffset;
	m_Size = newSize;
	m_Data = newData;
	m_DataSize = newDataSize;

	if (m_OnUpdateCallback != nullptr) {
		m_OnUpdateCallback();
	}
}

ng::vulkan::VulkanBuffer::VulkanBuffer()
{

}

ng::vulkan::VulkanBuffer::VulkanBuffer(const VulkanBuffer & buffer)
{
	m_BufferRegionAllocator = buffer.m_BufferRegionAllocator;
	m_Offset = buffer.m_Offset;
	m_Size = buffer.m_Size;
	m_DataSize = buffer.m_DataSize;
	m_VkBuffer = buffer.m_VkBuffer;
	m_Data = buffer.m_Data;
	m_OnUpdateCallback = buffer.m_OnUpdateCallback;
	
	m_BufferRegionAllocator->increaseBufferCopies(this);
}

ng::vulkan::VulkanBuffer::VulkanBuffer(VulkanBufferInternalCreateInfo createInfo)
{
	init(createInfo);
}

void ng::vulkan::VulkanBuffer::init(VulkanBufferInternalCreateInfo createInfo)
{
	m_BufferRegionAllocator = createInfo.bufferRegionAllocator;
	m_Offset = createInfo.offset;
	m_Size = createInfo.size;
	m_DataSize = createInfo.dataSize;
	m_VkBuffer = createInfo.vkBuffer;
	m_Data = createInfo.data;
	m_OnUpdateCallback = createInfo.onUpdateCallback;

	if (m_Data != nullptr) {
		write(m_Data, m_DataSize);
	}
}

ng::vulkan::VulkanBuffer::~VulkanBuffer()
{
	if (m_BufferRegionAllocator != nullptr) {
		m_BufferRegionAllocator->freeBuffer(this);
	}
}

ng::vulkan::VulkanBuffer ng::vulkan::VulkanBuffer::operator=(const VulkanBuffer & buffer)
{
	m_BufferRegionAllocator = buffer.m_BufferRegionAllocator;
	m_Offset = buffer.m_Offset;
	m_Size = buffer.m_Size;
	m_DataSize = buffer.m_DataSize;
	m_VkBuffer = buffer.m_VkBuffer;
	m_Data = buffer.m_Data;
	m_OnUpdateCallback = buffer.m_OnUpdateCallback;

	m_BufferRegionAllocator->increaseBufferCopies(this);
}

void ng::vulkan::VulkanBuffer::setUpdateCallbackFunc(void(*callbackFunc)())
{
	m_OnUpdateCallback = callbackFunc;
}

void ng::vulkan::VulkanBuffer::write(void * data, VkDeviceSize dataSize)
{
	m_BufferRegionAllocator->write(this, data, dataSize);
}

void ng::vulkan::VulkanBuffer::free()
{
	if (m_BufferRegionAllocator != nullptr) {
		m_BufferRegionAllocator->freeBuffer(this);
	}
}

VkBuffer ng::vulkan::VulkanBuffer::getBuffer()
{
	return m_VkBuffer;
}

VkDeviceSize ng::vulkan::VulkanBuffer::getOffset()
{
	return m_Offset;
}

VkDeviceSize ng::vulkan::VulkanBuffer::getDataSize()
{
	return m_DataSize;
}

VkDeviceSize ng::vulkan::VulkanBuffer::getBufferSize()
{
	return m_Size;
}

std::size_t ng::vulkan::VulkanBuffer::hash(VulkanBuffer const& buffer)
{
	std::size_t ret = std::hash<VkBuffer>()(buffer.m_VkBuffer);
	ng::math::hashCombine<VkDeviceSize>(ret, buffer.m_Offset);
	return ret;
}
