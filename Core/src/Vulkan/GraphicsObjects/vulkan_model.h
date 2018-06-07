#pragma once

#include "../../def.h"
#include "../Memory/vulkan_buffer.h"
#include "../vulkan_texture.h"

namespace ng {
	namespace vulkan {

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
			VulkanBuffer VIBO;

			std::vector<ModelPart> modelParts;

			ng::vulkan::VulkanTexture textureArray;
			
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

namespace std {
	template<> struct hash<ng::vulkan::VulkanModel> {
		size_t operator()(ng::vulkan::VulkanModel const& model) const {
			return ng::vulkan::VulkanModel::hash(model);
		}
	};
}