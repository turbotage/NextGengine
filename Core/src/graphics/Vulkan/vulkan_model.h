#pragma once

#include "../../def.h"
#include "vulkan_base.h"

namespace ng {
	namespace graphics {

		class VulkanModel
		{
			VulkanBase* m_VulkanBase;
			vk::DescriptorSetLayout* m_DescSetLayout;
			vk::DescriptorSet m_DescriptorSet;
		public:

			void init(VulkanBase* vulkanBase, vk::DescriptorSetLayout* descriptorSetLayout);

			void loadModel(const char* filename);
		};
	}
}
