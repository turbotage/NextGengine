#pragma once

#include "../../def.h"
#include "../../Memory/vulkan_buffer.h"
#include "../../Graphics/vulkan_texture.h"

namespace ng {
	namespace graphics {

		class VulkanModel
		{
		public:
			
			ng::memory::VulkanBuffer m_VertexAndIndexBuffer;
			
			struct ModelPart {
				uint32 vertexOffset;
				uint32 indexOffset;

				uint32 indexCount;

				ng::graphics::VulkanTexture textureArray;
			};

			std::vector<ModelPart> modelParts;
			
		public:
			
			void init(VulkanBase* vulkanBase, VkDescriptorSetLayout* descriptorSetLayout);

			void loadModel(const char* filename);

		};
	}

}

