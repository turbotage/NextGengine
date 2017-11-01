#pragma once

#include "../../def.h"
#include "../../Memory/VirtualMemoryAllocators/allocator.h"
#include "../vulkan_base.h"

namespace ng {
	namespace graphics {

		/*
		class Block {
			vk::DeviceMemory memory;
			vk::DeviceSize offset;
			vk::DeviceSize size;
			bool free = false;
			void* dataPtr = nullptr; //won't do anyting if this is a device local allocation

			bool operator==(Block const &block);
		};

		class Chunk {

		};
		*/

		class VulkanAllocator
		{
		public:

			enum AllocatorUsage {
				STATIC_STORAGE_BUFFER,
				DYNAMIC_STORAGE_BUFFER
			};

			enum DeviceType {
				GRAPHICS_UNIT,
				COMPUTE_UNIT
			};

			void init(VulkanBase* vkBase);
			
			static uint32 findMemoryType(VkPhysicalDevice* pDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

			void createBufferAndMemory(DeviceType deviceType, VkBuffer* buffer, VkDeviceMemory* memory, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkSharingMode sharingMode);

		private:
			VkDeviceSize m_GraphicsMemorySize;
			VkDeviceMemory m_GraphicsDeviceMemory;

			VkDeviceSize m_ComputeDeviceMemorySize;
			VkDeviceMemory m_ComputeDeviceMemory;

			VkDeviceSize m_HostLocalMemorySize;
			VkDeviceMemory m_HostLocalMemory;

			VkBuffer m_GraphicsBuffer;
			VkBuffer m_HostBuffer;
			VkBuffer m_ComputeBuffer;
			

			VulkanBase* m_VulkanBase;

			VkBuffer m_Buffer;

			std::vector<VkBuffer> stagingBuffers;

		};

	}
}
