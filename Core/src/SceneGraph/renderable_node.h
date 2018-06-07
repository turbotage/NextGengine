#pragma once

#include "scene_graph.h"
#include "../Vulkan/Pipelines/vulkan_graphics_pipeline.h"
#include "../BoundingVolumes/bounding_sphere.h"
#include "culling_walker.h"

namespace ng {
	namespace scenegraph {

		struct InstanceData {
			ng::math::Mat4f transformation;
			uint32 textureArrayIndex;
		};

		class RenderableNode : SceneNode
		{
			friend class CullingWalker;
		private:

			/**  RENDER-STATE : contains the vulkan-model **/
			std::list<ng::scenegraph::RenderState>::iterator m_RenderState;

			/**  TEXTURE-ARRAY-INDEX : the index that specifies which texture in the vulkan-model 
			texture-array that are to be used for rendering this node  **/
			uint32 m_TextureArrayIndex;

			/**  VULKAN-GRAPHICS-PIPELINE : the pipeline which will be used to render the vulkan-model  **/
			ng::vulkan::VulkanGraphicsPipeline* m_GraphicsPipeline;
			
			/**  RENDER-LOCK : set to false or true to enable or disable rendering  **/
			bool m_RenderingEnabled = true;

			/**  CULL-LOCK : is set to false if this node has already been culled, gets set to false when culling-walker goes over it  **/
			CullingFlags m_CullingFlags;

		protected:

			/**  BOUNDING-SPHERE  **/
			ng::bvolumes::BoundingSphere boundingSphere;

		public:


			RenderableNode();
			~RenderableNode();

			static RenderableNode* cast(SceneNode* node);

			InstanceData getInstanceData();

		};


	}
}
