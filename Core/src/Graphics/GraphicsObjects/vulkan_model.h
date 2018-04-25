#pragma once

#include "../../def.h"
#include "../../Memory/vulkan_buffer.h"
#include "../../Graphics/vulkan_texture.h"

namespace ng {
	namespace graphics {

		class VulkanModel
		{
		public: //types
			struct ModelPart {
				uint32 vertexOffset;
				uint32 indexOffset;

				uint32 indexCount;
			};
			
		public: //members
			
			/**  Vertex-Index Buffer Object  **/
			ng::memory::VulkanBuffer VIBO;

			std::vector<ModelPart> modelParts;

			ng::graphics::VulkanTexture textureArray;
			
			float hqLodDistance;
			float mqLodDistance;
			float lqLodDistance;

			
		public: //member functions
			
			void init(VulkanBase* vulkanBase, VkDescriptorSetLayout* descriptorSetLayout);

			void loadModel(const char* filename);

			VulkanModel operator=(const VulkanModel& model);

			static std::size_t hash(VulkanModel const& model);

		};
	}

}
