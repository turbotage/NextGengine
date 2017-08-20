#include "vulkan_allocator.h"


uint32 findMemoryType(uint32 memoryTypeBits, VkPhysicalDeviceMemoryProperties* properties, ng::graphics::VulkanAllocator::AllocatorUsage usage) {
	auto getMemoryType = [&](VkMemoryPropertyFlags propertyFlags) -> uint32 {
		for (uint32 i = 0; i < properties->memoryTypeCount; ++i) {
			if ((memoryTypeBits) & (1 << i) && ((properties->memoryTypes[i].propertyFlags) & propertyFlags)) {
				return i;
			}
		}
		return -1;
	};

	using namespace ng::graphics;
	switch (usage) {
	case VulkanAllocator::AllocatorUsage::STATIC_STORAGE_BUFFER:
		return getMemoryType(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		break;
	case VulkanAllocator::AllocatorUsage::DYNAMIC_STORAGE_BUFFER:
		{
			VkMemoryPropertyFlags optimal = 
				VK_MEMORY_PROPERTY_HOST_CACHED_BIT | 
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | 
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

			VkMemoryPropertyFlags required = 
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | 
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;


			uint32 type = getMemoryType(optimal);
			if (type == -1) {
				int result = getMemoryType(required);
				if (result == -1) {
					assert(!"Memory type not found");
				}
				return result;
			}
			return type;
			break;
		}
	default:
		{
		assert(!"Badly specified AllocatorUsage");
		}
		break;
	}
}

void ng::graphics::VulkanAllocator::init(VulkanBase * vkBase, AllocatorUsage allocUsage, DeviceType deviceType)
{
	m_AllocatorUsage = allocUsage;
	m_DeviceType = deviceType;
	VkPhysicalDeviceMemoryProperties* memProps;
	VkPhysicalDevice* pDevice;

	switch (m_DeviceType) {
	case GRAPHICS_UNIT:
		memProps = &m_VulkanBase->graphicsUnit.pDevice->memoryProperties;
		pDevice = &m_VulkanBase->graphicsUnit.pDevice->device;
		break;
	case COMPUTE_UNIT:
		memProps = &m_VulkanBase->computeUnit.pDevice->memoryProperties;
		pDevice = &m_VulkanBase->computeUnit.pDevice->device;
		break;
	}
	//*memProps = pDevice->getMemoryProperties();

	//vk::MemoryAllocateInfo allocInfo;

}

void ng::graphics::VulkanAllocator::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{

}
