#pragma once

#include "../def.h"
#include "../Math/vec3.h"
#include "../Entities/entity.h"
#include <mutex>

class Collidable
{
private:
	uint8 m_CollisionProps;
	ng::math::Vec3 m_Dimensions;

	ng::entity::Entity* m_Entity;
	std::mutex* m_Mutex;
public:

	enum eCollisionProps {
		COLLISION_ON = 1,
	};

	Collidable();
	Collidable(uint8 collisionProps);
};



