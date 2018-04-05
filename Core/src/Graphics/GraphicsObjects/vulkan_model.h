#pragma once

#include "../../def.h"
#include "../../Memory/vulkan_buffer.h"
#include "../../Graphics/vulkan_texture.h"

namespace ng {
	namespace graphics {

		class VulkanModel
		{
		private:
			
			ng::memory::VulkanBuffer m_VertexAndIndexBuffer;
			
			ng::graphics::VulkanTexture m_TextureArray;

			struct ModelPart {
				uint32 vertexOffset;
				uint32 indexOffset;

				uint32 indexCount;
			};

			std::vector<ModelPart> m_ModelParts;
			
		public:
			
			void init(VulkanBase* vulkanBase, VkDescriptorSetLayout* descriptorSetLayout);

			void loadModel(const char* filename);

		};
	}

}

