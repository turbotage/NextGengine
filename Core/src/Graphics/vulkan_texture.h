#pragma once

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

			void updateDescriptor();

			void destroy();

		};

		class VulkanTexture2D : public VulkanTexture {
			void loadFromFile(
				std::string filename,
				VulkanDevice* vulkanDevice,
				VkFormat format,
				VkQueue copyQueue,
				VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
				VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				bool forceLinear = false
				);
		};


		class VulkanTextureCubeMap : public VulkanTexture {

		};



	}
}



