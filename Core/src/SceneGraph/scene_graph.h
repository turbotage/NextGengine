#pragma once

#include "../def.h"

#include "scene_node.h"
#include "../Vulkan/Pipelines/vulkan_graphics_pipeline.h"

namespace ng {
	namespace scenegraph {
		class RenderableNode;
		class RenderState;
		
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
			friend class CullingWalker;
		private:

			SceneNode* m_RootNode;

			std::shared_mutex m_SceneGraphMutex;
		public:
			

		};
		
		class Scene {
		private:
			friend class CullingWalker;

			SceneGraph m_SceneGraph;

			/** RENDER-STATES : contains all the local renderstates that might be used to render a model, this is the blueprint to which
			all instances render calls will use **/
			std::list<ng::scenegraph::RenderState> m_RenderStates;

			/**  SCENE-RENDER-STATE : hash map sorted by which pipeline to render with, containing another hashmap sortered by which local renderstate
			to render, containging a vector of all the renderable nodes in 
			the scene-graph that is to be rendered with the first keyd pipeline and second keys local renderstate **/
			std::unordered_map<
				ng::vulkan::VulkanGraphicsPipeline*,
				std::unordered_map<
					std::list<ng::scenegraph::RenderState>::iterator,
					std::unordered_set<RenderableNode*>
				>
			> m_SceneRenderState;

			/**  SCENE-RENDER-STATE-MUTEX : lock around when   **/
			std::shared_mutex m_SceneRenderStateMutex;

			std::vector<RenderableNode*> m_Culled;

		};
		
	}
}



