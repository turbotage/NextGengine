#pragma once

#include "../../../def.h"
#include "../../../Memory/VirtualMemoryAllocators/allocator.h"
#include "../vulkan_base.h"

namespace ng {
	namespace graphics {

		class VulkanAllocator
		{
		public:

			enum AllocatorUsage {
				eStaticStorageBuffer,
				eDynamicStorageBuffer
			};

			enum DeviceType {
				eGraphicsUnit,
				eComputeUnit
			};

			void init(VulkanBase* vkBase, AllocatorUsage allocUsage, DeviceType deviceType);


			VulkanAllocator();
			~VulkanAllocator();

		private:

			memory::Allocator m_Allocator;

			VulkanBase* m_VulkanBase;

			std::vector<vk::Buffer> m_StagingBuffers;

			AllocatorUsage m_AllocatorUsage;
			DeviceType m_DeviceType;

			void createBuffer();
		};

	}
}
