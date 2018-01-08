#pragma once

#include "../def.h"
#include "../Math/Vec3f.h"
#include "../Entities/entity.h"
#include <mutex>

class Collidable
{
private:
	uint8 m_CollisionProps;
	ng::math::Vec3f m_Dimensions;

	ng::entity::Entity* m_Entity;
	std::mutex* m_Mutex;
public:

	enum eCollisionProps {
		COLLISION_ON = 1,
	};

	Collidable();
	Collidable(uint8 collisionProps);
};



