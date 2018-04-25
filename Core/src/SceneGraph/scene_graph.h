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
#include "../Math/hash_functions.h"

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

		public:

			static std::size_t hash(RenderState const& renderState) {
				return ng::graphics::VulkanModel::hash(renderState.model);
			}

			static std::size_t hash(std::list<RenderState>::iterator iter) {
				return iter->hash(*iter);
			}

		};

	}
}

namespace ng {
	namespace scenegraph {
		
		/**
		1 : cull the tree and prepare the scene-render-state
		2 : render the culled render-targets
		3 : collision checking, 
		**/

		class SceneGraph
		{
		private:
			friend CullingWalker;

			SceneNode* m_RootNode;

			std::shared_mutex m_SceneGraphMutex;
		public:
			

		};
		
		class Scene {
		private:
			friend CullingWalker;

			SceneGraph m_SceneGraph;

			/** RENDER-STATES : contains all the local renderstates that might be used to render a model, this is the blueprint to which
			all instances render calls will use **/
			std::list<RenderState> m_RenderStates;

			/**  SCENE-RENDER-STATE : hash map sorted by which pipeline to render with, containing another hashmap sortered by which local renderstate
			to render, containging a vector of all the renderable nodes in 
			the scene-graph that is to be rendered with the first keyd pipeline and second keys local renderstate **/
			std::unordered_map<
				ng::graphics::VulkanGraphicsPipeline*,
				std::unordered_map<
					std::list<ng::scenegraph::RenderState>::iterator,
					std::vector<RenderableNode*>
				>
			> m_SceneRenderState;

			/**  SCENE-RENDER-STATE-MUTEX : lock around when   **/
			std::shared_mutex m_SceneRenderStateMutex;

			std::vector<RenderableNode*> m_Culled;

		};
		
	}
}



