#pragma once

#include "../../../def.h"
#include "../../../Memory/VirtualMemoryAllocators/allocator.h"
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

			void init(VulkanBase* vkBase, AllocatorUsage allocUsage, DeviceType deviceType);

			VulkanAllocator();
			~VulkanAllocator();

		private:
			AllocatorUsage m_AllocatorUsage;
			DeviceType m_DeviceType;

			VulkanBase* m_VulkanBase;

			VkBuffer m_Buffer;

			void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
		};

	}
}
