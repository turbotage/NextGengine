#include "vulkan_buffer.h"
#include "vulkan_buffer_region_allocator.h"
#include "../../Math/hash_functions.h"

void ng::vulkan::VulkanBuffer::update()
{
	if (m_OnUpdate != nullptr) {
		m_OnUpdate();
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
	m_OnUpdate = buffer.m_OnUpdate;
	
	m_BufferRegionAllocator->increaseBufferCopies(this);
}

ng::vulkan::VulkanBuffer::VulkanBuffer(VulkanBufferCreateInfo createInfo)
{
	init(createInfo);
}

void ng::vulkan::VulkanBuffer::init(VulkanBufferCreateInfo createInfo)
{
	m_BufferRegionAllocator = createInfo.bufferRegionAllocator;
	m_Offset = createInfo.offset;
	m_Size = createInfo.size;
	m_DataSize = createInfo.dataSize;
	m_VkBuffer = createInfo.vkBuffer;
	m_Data = createInfo.data;
	m_OnUpdate = createInfo.callback;

	if (m_Data != nullptr) {

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
	m_OnUpdate = buffer.m_OnUpdate;

	m_BufferRegionAllocator->increaseBufferCopies(this);
}

void ng::vulkan::VulkanBuffer::setBufferUpdateCallbackFunc(void(*callbackFunc)())
{
	m_OnUpdate = callbackFunc;
}

void ng::vulkan::VulkanBuffer::write(void * data, VkDeviceSize nBytes, VkCommandBuffer writeBuffer)
{

}

void ng::vulkan::VulkanBuffer::free()
{
	if (m_BufferRegionAllocator != nullptr) {
		m_BufferRegionAllocator->freeBuffer(this);
	}
}

std::size_t ng::vulkan::VulkanBuffer::hash(VulkanBuffer const& buffer)
{
	std::size_t ret = std::hash<VkBuffer*>()(buffer.m_VkBuffer);
	ng::math::hashCombine<VkDeviceSize>(ret, buffer.m_Offset);
	return ret;
}
