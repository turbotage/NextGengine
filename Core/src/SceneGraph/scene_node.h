#pragma once

#include <vector>

#include "../Math/mat4f.h"
#include "../Props/movement_properties.h"
#include "../BoundingVolumes/aabb.h"
#include "../Graphics/GraphicsObjects/vulkan_model.h"

namespace ng {
	namespace scenegraph {

		class CullingWalker;

		class SceneNode {
		private:

			friend CullingWalker;

			void update(float time);

			/**  will rebuild the AABB in this node  **/
			void rebuildBoundingVolume();

			ng::bvolumes::AABB* m_OuterAABBs[6]; //0 :  maxX, 1 : minX, 2 : maxY, 3 : minY, 4 : maxZ, 5 : minZ  

		protected:
			/**  TREE-STRUCTURE  **/
			SceneNode * m_Parent = nullptr;

			std::vector<SceneNode*> m_Children;

			std::string m_Name;

			uint8 m_NodeType;

			/**  POSITIONAL  **/
			/**  world-transform  **/
			ng::math::Mat4f m_WorldTransform;

			/**  node-position  **/
			ng::math::Vec3f m_Position;


			/**  MOVEMENT  **/
			bool m_MovementEnabled;

			/**  local-rotation  **/
			ng::props::LocalRotation* m_LocalRotation = nullptr;

			/**  linear-movement  **/
			ng::props::LinearMovement* m_LinearMovement = nullptr;


			/**  BOUNDING-VOLUME  **/
			ng::bvolumes::AABB m_AABB;

			ng::bvolumes::AABB m_MinimumAABB;


			/**  will update the AABB tree from this node to the top **/
			void updateBoundingVolumes(ng::bvolumes::AABB* updatedAABB = nullptr, bool isLocal = false);

			virtual void onAddChild();

			virtual void onUpdate(float time);

		public:

			SceneNode();

			const ng::math::Vec3f& getPosition();

			/**  rotates this node and all its children around the rotationAxis by angle degrees **/
			const ng::math::Mat4f& rotate(const ng::math::Vec3f& rotationAxis, float angle, bool updateBV = true);
			/**  rotates this node and all its children  **/
			const ng::math::Mat4f& rotate(const ng::math::Mat4f& rotationMatrix, bool updateBV = true);
			/**  rotates this node and all its children  **/
			const ng::math::Mat4f& rotate(const ng::math::Quaternion& rotationQuaternion, bool updateBV = true);
			/**  rotates this node and all its children around a point and axis by angle degrees  **/
			const ng::math::Mat4f& rotateAround(const ng::math::Vec3f& rotationPoint, const ng::math::Vec3f& rotationAxis, float angle, bool updateBV = true);

			/**  translates this node and all its children  **/
			const ng::math::Mat4f& translate(const ng::math::Vec3f& translation, bool updateBV = true);
			/**  translates this node and all its children  **/
			const ng::math::Mat4f& translate(const ng::math::Mat4f& translationMatrix, bool updateBV = true);

			/**  applies some linear transformation to this node and all its children  **/
			const ng::math::Mat4f& transform(const ng::math::Mat4f& transformation, bool updateBV = true);

			/**  adds a child to the node  **/
			void addChild(SceneNode* childNode);

			/**  gets called when cull-walker checks if this node is in camera view, 
				overide if things need to be done during frustrum culling  **/
			virtual void onFrustrumCulling();

			/**  gets called when collision-walker checks if this node has been in a collision,
			overide if things need to be done during collision-check  **/
			virtual void onCollisionCheck();

			/**  gets called when transformation-walker updates this node's position and movement,
			overide if things need to be done during transformation-changes  **/
			virtual void onMovementUpdate();

		};
	}
}

