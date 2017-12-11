#include "uniform_grid.h"
#include <math.h>

ng::dstructs::GridIndex ng::dstructs::UniformGrid::getIndex(ng::math::Vec3 position)
{
	GridIndex ret;
	if (position.x >= 0) {
		ret.x = ((position.x / m_GridSize) + 1);
	}
	else {
		ret.x = ((position.x / m_GridSize) - 1);
	}
	if (position.y >= 0) {
		ret.y = ((position.y / m_GridSize) + 1);
	}
	else {
		ret.y = ((position.y / m_GridSize) - 1);
	}
	if (position.z >= 0) {
		ret.z = ((position.z / m_GridSize) + 1);
	}
	else {
		ret.z = ((position.z / m_GridSize) - 1);
	}
	return ret;
}

ng::math::Vec3 ng::dstructs::UniformGrid::getCenterPosition(const GridIndex * index)
{
	ng::math::Vec3 ret;
	if (index->x >= 0) {
		ret.x = (index->x * m_GridSize) - (m_GridSize / 2);
	}
	else {
		ret.x = (index->x * m_GridSize) + (m_GridSize / 2);
	}
	if (index->y >= 0) {
		ret.y = (index->x * m_GridSize) - (m_GridSize / 2);
	}
	else {
		ret.y = (index->x * m_GridSize) + (m_GridSize / 2);
	}
	if (index->z >= 0) {
		ret.z = (index->x * m_GridSize) - (m_GridSize / 2);
	}
	else {
		ret.z = (index->x * m_GridSize) + (m_GridSize / 2);
	}
	return ret;
}

std::vector<Renderable*>* ng::dstructs::UniformGrid::frustrumCull(Camera & cam)
{
	m_ObjectsToRender.clear();
	m_GridIndicesToRender.clear();

	

}

ng::dstructs::UniformGrid::UniformGrid()
{
	
}

ng::dstructs::UniformGrid::UniformGrid(ng::math::Vec3s worldSizeMultiplier, float gridSize, uint16 expectedNumOfObjects)
	: m_GridSize(gridSize), m_WorldSizeMultipler(worldSizeMultiplier)
{
	float t = gridSize * 0.5;
	m_GridIndexRadius = sqrt(3 * t * t);
}
