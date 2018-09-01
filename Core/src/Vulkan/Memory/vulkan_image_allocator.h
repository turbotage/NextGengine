#pragma once

#include "../vulkan_device.h"
#include "vulkan_image.h"

namespace ng {
	namespace vulkan {

		class VulkanImageAllocator {
		private:
			friend class VulkanImage;
			friend class VulkanTexture2D;
			friend class VulkanTextureArray;

			VulkanDevice* m_VulkanDevice;

			std::list<VulkanImage*> m_Images;

			std::list<VulkanImageAllocation> m_Allocations;

		public:
			
			VulkanImageAllocator(VulkanDevice* vulkanDevice);

			VulkanImageAllocator(const VulkanImageAllocator& other) = delete;

			VulkanImageAllocator(VulkanImageAllocator &&) = delete;

			void createTexture2D(VulkanImageCreateInfo createInfo, VulkanTexture2D* image);

			void createTextureArray(VulkanImageCreateInfo createInfo, VulkanTextureArray* image);

			void freeImage(VulkanImage* image);

		};

	}
}