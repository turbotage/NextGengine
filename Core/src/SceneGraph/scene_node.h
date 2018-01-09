#pragma once

#include <vector>

#include "../Math/mat4.h"
#include "../Props/movement_properties.h"
#include "../BoundingVolumes/bounding_sphere.h"

namespace ng {
	namespace scenegraph {
		class SceneNode {
		private:

		protected:
			/**  TREE-STRUCTURE  **/
			SceneNode * m_Parent = nullptr;
			std::vector<SceneNode*> m_Children;
			std::string m_Name;

			/**  POSITIONAL  **/
			/**  world-transform  **/
			ng::math::Mat4 m_WorldTransform;

			/**  node-position  **/
			ng::math::Vec3f m_Position;


			/**  MOVEMENT  **/
			ng::props::mMovementPropertiesMask m_MovementProperties;

			/**  point-rotation  **/
			ng::props::PointRotation* m_PointRotation = nullptr;

			/**  local-rotation  **/
			ng::props::LocalRotation* m_LocalRotation = nullptr;

			/**  linear-movement  **/
			ng::props::LinearMovement* m_LinearMovement = nullptr;


			/**  BOUNDING-VOLUME  **/
			ng::bvolumes::BoundingSphere m_BoundingSphere;

			virtual void onAddChild();

			virtual void onUpdate(float time);

		public:

			void updateBoundingVolumes();

			SceneNode();

			const ng::math::Vec3f& getCenterPosition();

			const ng::math::Vec3f& getBoundingSpherePosition();

			float getBoundingSphereRadius() const;
			void setBoundingSphereRadius(float radius);

			bool hasMovementProperties(ng::props::mMovementPropertiesMask propsMask);
			void addMovementProperties(ng::props::mMovementPropertiesMask propsMask);
			void setMovementProperties(ng::props::mMovementPropertiesMask propsMask);

			/**  rotates this node and all its children around the rotationAxis by angle degrees **/
			const ng::math::Mat4& rotate(const ng::math::Vec3f& rotationAxis, const float angle);
			/**  rotates this node and all its children  **/
			const ng::math::Mat4& rotate(const ng::math::Mat4& rotationMatrix);
			/**  rotates this node and all its children  **/
			const ng::math::Mat4& rotate(const ng::math::Quaternion& rotationQuaternion);
			/**  rotates this node and all its children around a point and axis by angle degrees  **/
			const ng::math::Mat4& rotateAround(const ng::math::Vec3f& rotationPoint, const ng::math::Vec3f& rotationAxis, const float angle);

			/**  translates this node and all its children  **/
			const ng::math::Mat4& translate(const ng::math::Vec3f& translation);
			/**  translates this node and all its children  **/
			const ng::math::Mat4& translate(const ng::math::Mat4& translationMatrix);

			/**  applies some linear transformation to this node and all its children  **/
			const ng::math::Mat4& transform(const ng::math::Mat4& transformation);

			/*  adds a child to the node  */
			void addChild(SceneNode* childNode);

			void update(float time);

		};
	}
}

