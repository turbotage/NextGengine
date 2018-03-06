#pragma once

#include "../../def.h"
#include "../../Memory/buffer.h"

namespace ng {
	namespace graphics {

		class VulkanModel
		{
		public:
			ng::memory::Buffer* memoryBuffer;
			
			struct ModelPart {
				uint32 vertexOffset;
				uint32 vertexCount;

				uint32 indexOffset;
				uint32 indexCount;
			};

			std::vector<ModelPart>* modelParts;

			VkDescriptorSetLayout* m_DescSetLayout;
			
			VkDescriptorSet m_DescriptorSet;
			
		public:
			
			void init(VulkanBase* vulkanBase, VkDescriptorSetLayout* descriptorSetLayout);

			void loadModel(const char* filename);
		};
	}
}
