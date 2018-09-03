#pragma once

#include "../../def.h"
#include "../Memory/vulkan_buffer.h"
#include "../vulkan_texture.h"

namespace ng {
	namespace vulkan {

		struct ModelPart {
			uint32 vertexOffset;
			uint32 indexOffset;

			uint32 indexCount;
		};

		class VulkanModel
		{
		private: //members
			
			/**  Vertex-Index Buffer Object  **/
			VulkanBuffer VIBO;

			std::vector<ModelPart> modelParts;

			/* is -1 if this is a single layer texture, 
			otherwise it specifies which layer in the texture-array the models texture is */
			uint32 textureLayer;
			/* can be cast to VulkanTexture2D or VulkanTextureArray*/
			ng::vulkan::VulkanTexture texture;
			
			std::vector<float> lodDistances;

			
		public: //member functions
			
			void init(VulkanBase* vulkanBase, VkDescriptorSetLayout* descriptorSetLayout);

			void loadModel(const char* filename);

			VulkanModel operator=(const VulkanModel& model);

			static std::size_t hash(VulkanModel const& model);

		};
	}
}

namespace std {
	template<> struct hash<ng::vulkan::VulkanModel> {
		size_t operator()(ng::vulkan::VulkanModel const& model) const {
			return ng::vulkan::VulkanModel::hash(model);
		}
	};
}