#pragma once

#include "scene_graph.h"
#include "camera_node.h"

namespace ng {
	namespace scenegraph {

#define HAS_BEEN_CULLED 1 //if the model has already been culled in pre-culling
#define NOTHING_IN_FRUSTRUM 2 //if neither model nor aabb is in frustrum
#define EVERYTHING_IN_FRUSTRUM 4 //whole aabb is in the frustrum therefore also model is in frustrum
#define MODEL_IN_FRUSTRUM 8 //if the model is in frustrum but not the entire aabb
#define AABB_IN_FRUSTRUM 16 //if the aabb is in the frustrum, this does not imply that model is frustrum

		/**  Read the macros to understand the functions  **/
		class CullingFlags {
		private:
			uint8 m_CullingFlags;
		public:

			void setBeenCulledTrue() {
				m_CullingFlags |= 1;
			}

			void setBeenCulledFalse() {
				m_CullingFlags &= ~1;
			}

			bool hasBeenCulled() {
				return ((m_CullingFlags) & 1);
			}

			void setFlags(uint8 flags) {
				m_CullingFlags = flags;
			}

			uint8 getFlags() {
				return m_CullingFlags;
			}

			bool nothingInFrustrum() {
				return ((m_CullingFlags >> 1) & 1U);
			}

			bool allInFrustrum() {
				return ((m_CullingFlags >> 2) & 1U);
			}

			bool modelInFrustrum() {
				return ((m_CullingFlags >> 3) & 1U);
			}

			bool aabbInFrustrum() {
				return ((m_CullingFlags >> 4) & 1U);
			}
			
		};

		class CullingWalker
		{
		private:

			Scene* m_Scene;

			std::map<ng::math::Vec3f, RenderableNode*>* m_ToBeRenderedPtr;

			void addToRendering(ng::scenegraph::RenderableNode* node);

			void removeFromRendering(ng::scenegraph::RenderableNode* node);

			void addToRenderingRecursively(ng::scenegraph::RenderableNode* node);

			void removeFromRenderingRecursively(ng::scenegraph::RenderableNode* node);

			CullingFlags isInView(RenderableNode* node, CameraNode* camera);

		public:

			CullingWalker();
			CullingWalker(Scene* scene);

			~CullingWalker();

			void walk(CameraNode* camera);
		};
	}
}


