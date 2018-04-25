#include "vulkan_buffer.h"
#include "vulkan_buffer_region_allocator.h"
#include "../Math/hash_functions.h"

namespace ng {
	namespace memory {

		void VulkanBuffer::update()
		{
			if (onUpdate != nullptr) {
				onUpdate();
			}
		}

		VulkanBuffer::VulkanBuffer()
		{

		}

		VulkanBuffer::VulkanBuffer(VulkanBufferCreateInfo createInfo)
		{
			init(createInfo);
		}

		void VulkanBuffer::init(VulkanBufferCreateInfo createInfo)
		{
			m_Offset = createInfo.offset;
			m_Size = createInfo.size;
			m_VkBuffer = createInfo.vkBuffer;
			m_Data = malloc(m_Size);
			m_BufferRegionAllocator = createInfo.bufferRegionAllocator;
		}

		VulkanBuffer::~VulkanBuffer()
		{
			if (m_BufferRegionAllocator != nullptr) {
				m_BufferRegionAllocator->freeBuffer(this);
			}
		}

		VulkanBuffer VulkanBuffer::operator=(const VulkanBuffer & buffer)
		{
			m_BufferRegionAllocator = buffer.m_BufferRegionAllocator;
			m_Data = buffer.m_Data;
			m_VkBuffer = buffer.m_VkBuffer;
			m_Offset = buffer.m_Offset;
			m_Size = buffer.m_Size;
			m_BufferRegionAllocator->increaseBufferCopies(this);
		}

		void VulkanBuffer::setBufferUpdateCallbackFunc(void(*callbackFunc)())
		{
			onUpdate = callbackFunc;
		}

		void VulkanBuffer::free()
		{
			m_BufferRegionAllocator->freeBuffer(this);
		}

		std::size_t VulkanBuffer::hash(VulkanBuffer const& buffer)
		{
			std::size_t ret = std::hash<VkBuffer*>()(buffer.m_VkBuffer);
			ng::math::hashCombine<VkDeviceSize>(ret, buffer.m_Offset);
			return ret;
		}

	}
}