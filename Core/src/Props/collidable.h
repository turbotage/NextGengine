#pragma once

#include "../BoundingVolumes/general_bv.h"
#include "../def.h"

template <typename T>
class Collidable
{
private:

	uint8 m_CollisionProps;
	T* m_DerivedParent;
public:

	enum eCollisionProps {
		COLLISION_ON = 1,
	};

	GeneralBV boundingVolume;

	Collidable();
	Collidable(uint8 collisionProps);

protected:
	Collidable(T* derivedParent);
	Collidable(uint8 collisionProps, T* derivedParent);
};

template<typename T>
inline Collidable<T>::Collidable()
{

}

template<typename T>
inline Collidable<T>::Collidable(uint8 collisionProps)
{
	m_CollisionProps = collisionProps;
}

template<typename T>
inline Collidable<T>::Collidable(T * derivedParent)
{
	m_DerivedParent = derivedParent;
}

template<typename T>
inline Collidable<T>::Collidable(uint8 collisionProps, T * derivedParent)
{
	m_CollisionProps = collisionProps;
	m_DerivedParent = derivedParent;
}

