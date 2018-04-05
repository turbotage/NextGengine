#pragma once

#include "../def.h"

#include "../Math/mat4f.h"
#include "../Math/quaternion.h"
#include "../Entities/mesh.h"
#include "../BoundingVolumes/general_bv.h"
#include "../BoundingVolumes/bounding_sphere.h"
#include "../Props/movement_properties.h"
#include "scene_node.h"
#include "renderable_node.h"

#include "../Graphics/GraphicsObjects/vulkan_model.h"
#include "../Graphics/Pipelines/vulkan_graphics_pipeline.h"

namespace ng {
	namespace scenegraph {

		class RenderState {
		private:
			uint32 m_Instances;
		public:
			/**  VULKAN-MODEL : vulkan-model  **/
			ng::graphics::VulkanModel model;
		};
		
		class SceneGraph
		{
		private:
			friend CullingWalker;

			SceneNode* m_RootNode;
		public:
			

		};

		class Scene {
		private:
			friend CullingWalker;

			std::list<RenderState> m_RenderStates;

			std::map<ng::math::Vec3f, RenderableNode*> m_ToBeRendered;

			SceneGraph m_SceneGraph;



		};
		
	}
}



