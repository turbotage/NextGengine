#pragma once

#include <gli\gli.hpp>

#include "vulkan_device.h"

namespace ng {
	namespace graphics {

		class VulkanTexture
		{
		private:

		public:
			VulkanDevice* vulkanDevice;

			VkImage image;
			VkImageLayout imageLayout;
			VkImageView view;

			uint32 width, height;
			uint32 mipLevels;
			uint32 layerCount;

			VkDescriptorImageInfo descriptor;

			VkSampler sampler;

		public:
			VulkanTexture();
			~VulkanTexture();

			void updateDescriptor();

			void destroy();

		};



		class VulkanTextureCubeMap : public VulkanTexture {

		};



	}
}



