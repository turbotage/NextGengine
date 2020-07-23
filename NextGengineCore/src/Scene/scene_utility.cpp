#include "scene_utility.h"

ng::AABB3D::AABB3D(glm::vec3 min, glm::vec3 max)
	: m_Min(min), m_Max(max)
{

}

glm::vec3 ng::AABB3D::getMin()
{
	return m_Min;
}

glm::vec3 ng::AABB3D::getMax()
{
	return m_Max;
}

void ng::AABB3D::setMin(glm::vec3 min)
{
	m_Min = min;
}

void ng::AABB3D::setMax(glm::vec3 max)
{
	m_Max = max;
}

ng::AABB2D::AABB2D(glm::vec2 min, glm::vec2 max)
	: m_Min(min), m_Max(max)
{
}

glm::vec2 ng::AABB2D::getMin()
{
	return m_Min;
}

glm::vec2 ng::AABB2D::getMax()
{
	return m_Max;
}

void ng::AABB2D::setMin(glm::vec2 min)
{
	min = m_Min;
}

void ng::AABB2D::setMax(glm::vec2 max)
{
	max = m_Max;
}
