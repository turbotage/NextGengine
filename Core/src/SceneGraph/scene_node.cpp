#include "scene_node.h"

void ng::scenegraph::SceneNode::onAddChild()
{

}

void ng::scenegraph::SceneNode::onUpdate(float time)
{
	
}

void ng::scenegraph::SceneNode::rebuildBoundingVolume()
{
	float xmin, xmax;
	float ymin, ymax;
	float zmin, zmax;

	xmax = m_MinimumAABB.maxX();
	xmin = m_MinimumAABB.minX();
	ymax = m_MinimumAABB.maxY();
	ymin = m_MinimumAABB.minY();
	zmax = m_MinimumAABB.maxZ();
	zmin = m_MinimumAABB.minZ();

	for (int i = 0; i <m_Children.size(); ++i) {
		if (m_Children[i]->m_AABB.maxX() > xmax) {
			xmax = m_Children[i]->m_AABB.maxX();
		}
		if (m_Children[i]->m_AABB.minX() < xmin) {
			xmin = m_Parent->m_Children[i]->m_AABB.minX();
		}
		if (m_Children[i]->m_AABB.maxY() > ymax) {
			ymax = m_Children[i]->m_AABB.maxY();
		}
		if (m_Children[i]->m_AABB.minY() < ymin) {
			ymin = m_Children[i]->m_AABB.minY();
		}
		if (m_Children[i]->m_AABB.maxZ() > zmax) {
			zmax = m_Children[i]->m_AABB.maxZ();
		}
		if (m_Children[i]->m_AABB.minZ() < zmin) {
			zmin = m_Children[i]->m_AABB.minZ();
		}
	}

	m_AABB.setMaxX(xmax);
	m_AABB.setMinX(xmin);
	m_AABB.setMaxY(ymax);
	m_AABB.setMinY(ymin);
	m_AABB.setMaxZ(zmax);
	m_AABB.setMinZ(zmin);
}

void ng::scenegraph::SceneNode::updateBoundingVolumes(ng::bvolumes::AABB* updatedAABB = nullptr, bool isLocal = false)
{
	//if this was the first bv to change just call parent bv change and then return
	if (isLocal) {
		//For now make all AABBs so big that all meshes can rotate without reWorking AABB should be fixed for later
		if (m_Parent != nullptr) {
			m_Parent->updateBoundingVolumes(&(this->m_AABB));
		}
		return;
	}

	//if a pointer to the updated AABB exists we can maybe speed up the AABB rebuild process
	if (updatedAABB != nullptr) {
		//if the changed aabb is one of the outer ones in this aabb then we have to redo this aabb and then call parent change bv
		for (int i = 0; i < 6; ++i) {
			if (updatedAABB == m_OuterAABBs[i]) {
				rebuildBoundingVolume();
				if (m_Parent != nullptr) {
					m_Parent->updateBoundingVolumes(&(this->m_AABB));
				}
				return;
			}
		}

		//if we now which one has changed just check if it is outside current aabb, if it is outside resize aabb if it isn't return without changes
		if (updatedAABB != nullptr) {
			bool hasChanged = false;
			if (updatedAABB->maxX() > m_AABB.maxX()) {
				hasChanged = true;
				m_AABB.setMaxX(updatedAABB->maxX());
				m_OuterAABBs[0] = updatedAABB;
			}
			if (updatedAABB->minX() < m_AABB.minX()) {
				hasChanged = true;
				m_AABB.setMinX(updatedAABB->minX());
				m_OuterAABBs[1] = updatedAABB;
			}
			if (updatedAABB->maxY() > m_AABB.maxY()) {
				hasChanged = true;
				m_AABB.setMaxY(updatedAABB->maxY());
				m_OuterAABBs[2] = updatedAABB;
			}
			if (updatedAABB->minY() < m_AABB.minY()) {
				hasChanged = true;
				m_AABB.setMinY(updatedAABB->minY());
				m_OuterAABBs[3] = updatedAABB;
			}
			if (updatedAABB->maxZ() > m_AABB.maxZ()) {
				hasChanged = true;
				m_AABB.setMaxZ(updatedAABB->maxZ());
				m_OuterAABBs[4] = updatedAABB;
			}
			if (updatedAABB->minZ() < m_AABB.minZ()) {
				hasChanged = true;
				m_AABB.setMinZ(updatedAABB->minZ());
				m_OuterAABBs[5] = updatedAABB;
			}
			if (hasChanged && (m_Parent != nullptr)) {
				m_Parent->updateBoundingVolumes(&(this->m_AABB));
			}
			return;
		}
	}

	//if this AABB shoud possibly change but we don't know which child node that has changed, this shoudl rarely happen!! if this happen often
	//restructure program since this iterative aproach is performance costly

	rebuildBoundingVolume();
	if (m_Parent != nullptr) {
		m_Parent->updateBoundingVolumes();
	}
}

ng::scenegraph::SceneNode::SceneNode()
{

}

const ng::math::Vec3f& ng::scenegraph::SceneNode::getPosition()
{
	return m_Position;
}

const ng::math::Mat4 & ng::scenegraph::SceneNode::rotate(const ng::math::Vec3f & rotationAxis, const float angle, bool updateBV = true)
{
	m_WorldTransform *= ng::math::Mat4::rotation(rotationAxis, angle);
	if (updateBV) {
		updateBoundingVolumes(&this->m_AABB, true);
	}
	return m_WorldTransform;
}

const ng::math::Mat4 & ng::scenegraph::SceneNode::rotate(const ng::math::Mat4 & rotationMatrix, bool updateBV = true)
{
	m_WorldTransform *= rotationMatrix;
	if (updateBV) {
		updateBoundingVolumes(&this->m_AABB, true);
	}
	return m_WorldTransform;
}

const ng::math::Mat4 & ng::scenegraph::SceneNode::rotate(const ng::math::Quaternion & rotationQuaternion, bool updateBV = true)
{
	m_WorldTransform *= ng::math::Mat4::rotation(rotationQuaternion);
	if (updateBV) {
		updateBoundingVolumes(&this->m_AABB, true);
	}
	return m_WorldTransform;
}

const ng::math::Mat4 & ng::scenegraph::SceneNode::rotateAround(const ng::math::Vec3f & rotationPoint, const ng::math::Vec3f & rotationAxis, const float angle, bool updateBV = true)
{
	// T(x, y, z) * R * T(-x, -y, -z)
	m_WorldTransform *= ng::math::Mat4::translation(rotationPoint);
	m_WorldTransform *= ng::math::Mat4::rotation(rotationAxis, angle);
	m_WorldTransform *= ng::math::Mat4::translation(-1.0f*rotationPoint);
	if (updateBV) {
		updateBoundingVolumes(&this->m_AABB, true);
	}
	return m_WorldTransform;
}

const ng::math::Mat4 & ng::scenegraph::SceneNode::translate(const ng::math::Vec3f & translation, bool updateBV = true)
{
	m_WorldTransform *= ng::math::Mat4::translation(translation);
	if (updateBV) {
		updateBoundingVolumes(&this->m_AABB, true);
	}
	return m_WorldTransform;
}

const ng::math::Mat4 & ng::scenegraph::SceneNode::translate(const ng::math::Mat4 & translationMatrix, bool updateBV = true)
{
	m_WorldTransform *= translationMatrix;
	if (updateBV) {
		updateBoundingVolumes(&this->m_AABB, true);
	}
	return m_WorldTransform;
}

const ng::math::Mat4 & ng::scenegraph::SceneNode::transform(const ng::math::Mat4 & transformation, bool updateBV = true)
{
	m_WorldTransform *= transformation;
	if (updateBV) {
		updateBoundingVolumes(&this->m_AABB, true);
	}
	return m_WorldTransform;
}

void ng::scenegraph::SceneNode::addChild(SceneNode * childNode)
{
	m_Children.push_back(childNode);
	onAddChild();
}

void ng::scenegraph::SceneNode::update(float time)
{
	if (m_MovementEnabled) {
		if (m_LocalRotation != nullptr) {
			rotate(m_LocalRotation->rotationAxis, m_LocalRotation->angularVelocity * time, false);
			m_LocalRotation->angularVelocity += (m_LocalRotation->angularAcceleration * time);
			m_LocalRotation->angularAcceleration += (m_LocalRotation->angularJerk * time);
		}
		if (m_LinearMovement != nullptr ) {
			translate(ng::math::Mat4::translation(m_LinearMovement->velocity * time));
			m_LinearMovement->velocity = m_LinearMovement->acceleration * time;
			m_LinearMovement->acceleration = m_LinearMovement->jerk * time;
		}
	}
	onUpdate(time);
}

/*
void ng::scenegraph::SceneNode::setCombinedCenter()
{
	m_BoundingSphere.centerPos = ng::math::Vec3f(0.0f, 0.0f, 0.0f);

	ng::math::Vec3f dirVec;
	for (int i = 0; i < m_Children.size(); ++i) {
		m_BoundingSphere.centerPos += (m_Children[i]->getCenterPosition() * m_Children[i]->getBoundingSphereRadius());
	}

	m_BoundingSphere.centerPos /= (float)m_Children.size();
}
*/

