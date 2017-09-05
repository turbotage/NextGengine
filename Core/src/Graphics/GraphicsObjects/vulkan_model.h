#pragma once

#include "../../def.h"
#include "../vulkan_base.h"

namespace ng {
	namespace graphics {

		class VulkanModel
		{
			VulkanBase* m_VulkanBase;
			VkDescriptorSetLayout* m_DescSetLayout;
			VkDescriptorSet m_DescriptorSet;
		public:

			void init(VulkanBase* vulkanBase, VkDescriptorSetLayout* descriptorSetLayout);

			void loadModel(const char* filename);
		};
	}
}
