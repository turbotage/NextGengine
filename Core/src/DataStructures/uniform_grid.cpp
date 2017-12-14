#include "uniform_grid.h"
#include <math.h>

short getQuadrant(float value) {
	if ((value >= -NG_PI / 4.0f) || (value < NG_PI / 4.0f)) {
		return 1;
	}
	if ((value < -3.0f*NG_PI / 4.0f) || (value >= 3.0f*NG_PI / 4.0f)) {
		return 2;
	}
	if ((value >= NG_PI / 4.0f) && (value < 3.0f*NG_PI / 4.0f)) {
		return 3;
	}
	if ((value < -NG_PI / 4.0f) && (value >= -3.0f*NG_PI / 4.0f)) {
		return 4;
	}
}

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
#define Q1 1
#define Q2 2
#define Q3 3
#define Q4 4

	m_ObjectsToRender.clear();
	m_GridIndicesToRender.clear();

	__m128 sidePlaneA =
		_mm_set_ps(cam.rightPlane.a, cam.leftPlane.a, cam.topPlane.a, cam.bottomPlane.a);
	__m128 sidePlaneB =
		_mm_set_ps(cam.rightPlane.b, cam.leftPlane.b, cam.topPlane.b, cam.bottomPlane.b);
	__m128 sidePlaneC =
		_mm_set_ps(cam.rightPlane.c, cam.leftPlane.c, cam.topPlane.c, cam.bottomPlane.c);
	__m128 sidePlaneD =
		_mm_set_ps(cam.rightPlane.d, cam.leftPlane.d, cam.topPlane.d, cam.bottomPlane.d);

	ng::math::Vec3 eulerAngles = cam.m_Rotation.EulerAngles(); //pitch, roll, yaw

	/*
	-	  Q2	-
	  -	      -
	    -	-
	Q3	  -	    Q1
		-	-
	  -		  -
	-	  Q4	-

	

	*/

	short pitchQuad = getQuadrant(eulerAngles.x);
	short yawQuad = getQuadrant(eulerAngles.z);

	switch (pitchQuad) {
	case Q1:
		
		break;
	case Q2:

		break;
	case Q3:

		break;
	case Q4:

		break;
	}
#undef Q1
#undef Q2
#undef Q3
#undef Q4
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
