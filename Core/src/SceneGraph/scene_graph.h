#pragma once

#include "../def.h"

#include "../Math/mat4f.h"
#include "../Math/quaternion.h"
#include "../Entities/mesh.h"
#include "../BoundingVolumes/general_bv.h"
#include "../BoundingVolumes/bounding_sphere.h"
#include "../Props/movement_properties.h"
#include "scene_node.h"

#include "../Graphics/GraphicsObjects/vulkan_model.h"

namespace ng {
	namespace scenegraph {

		struct RenderState {
			ng::graphics::VulkanModel* vulkanModel;
		};

		class Scene {

			std::list<ng::graphics::VulkanModel> m_Models;

			std::vector<RenderableNode*> m_ToBeRendered;

			SceneGraph m_SceneGraph;

		};
		
		class SceneGraph
		{
		private:
			SceneNode m_RootNode;
		public:
			

		};
		
	}
}



