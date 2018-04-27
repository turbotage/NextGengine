#include "culling_walker.h"
#include "scene_graph.h"
#include "renderable_node.h"

#include <immintrin.h>

/*
distance checking
	ng::bvolumes::AABB* aabb = &node->m_AABB;

	ng::math::Vec3f p0, p1, p2, p3, p4, p5, p6, p7;

	//top4
	__m128 bot4X = _mm_set_ps(p4.x, p5.x, p6.x, p7.x);
	__m128 bot4Y = _mm_set_ps(p4.y, p5.y, p6.y, p7.y);
	__m128 bot4Z = _mm_set_ps(p4.z, p5.z, p6.z, p7.z);
	//bot4
	__m128 top4X = _mm_set_ps(p0.x, p1.x, p2.x, p3.x);
	__m128 top4Y = _mm_set_ps(p0.y, p1.y, p2.y, p3.y);
	__m128 top4Z = _mm_set_ps(p0.z, p1.z, p2.z, p3.z);


	p0 = aabb->getPoint0();
	p1 = aabb->getPoint1();
	p2 = aabb->getPoint2();
	p3 = aabb->getPoint3();
	p4 = aabb->getPoint4();
	p5 = aabb->getPoint5();
	p6 = aabb->getPoint6();
	p7 = aabb->getPoint7();

	float a;
	float b;
	float c;
	float d;

	float rsqrt;

	__m128 planeA;
	__m128 planeB;
	__m128 planeC;
	__m128 planeD;

	ALIGN(16) float d1, d2, d3, d4;

	a = camera->leftPlane.a;
	b = camera->leftPlane.b;
	c = camera->leftPlane.c;
	d = camera->leftPlane.d;

	planeA = _mm_set_ps(a, a, a, a);
	planeB = _mm_set_ps(b, b, b, b);
	planeC = _mm_set_ps(c, c, c, c);
	planeD = _mm_set_ps(d, d, d, d);

	rsqrt = a * a + b * b + c * c;

	_mm_store_ps(&d1,
		_mm_rsqrt_ps(
			_mm_div_ps(
				_mm_add_ps(
					_mm_add_ps(
						_mm_mul_ps(top4X, planeA),
						_mm_mul_ps(top4Y, planeB)
					),
					_mm_add_ps(
						_mm_mul_ps(top4Z, planeC),
						planeD
					)
				),
				_mm_set_ps(rsqrt, rsqrt, rsqrt, rsqrt)
			)
		)
	);
*/

void ng::scenegraph::CullingWalker::addToRendering(ng::scenegraph::RenderableNode * node)
{
	auto it = m_Scene->m_SceneRenderState.find(node->m_GraphicsPipeline);
	if (it == m_Scene->m_SceneRenderState.end()) {

		std::unordered_map<std::list<ng::scenegraph::RenderState>::iterator, std::vector<RenderableNode*>> temp;
		temp.emplace(node->m_RenderState, node);

		m_Scene->m_SceneRenderState.insert(std::make_pair(node->m_GraphicsPipeline, temp));
	}
	else {
		std::unordered_map<std::list<ng::scenegraph::RenderState>::iterator, std::vector<RenderableNode*>> temp  = it->second;
		temp.find(node->m_RenderState);
	}
}

void ng::scenegraph::CullingWalker::removeFromRendering(ng::scenegraph::RenderableNode * node)
{

}

void ng::scenegraph::CullingWalker::addToRenderingRecursively(ng::scenegraph::RenderableNode * node)
{
	
}

void ng::scenegraph::CullingWalker::removeFromRenderingRecursively(ng::scenegraph::RenderableNode * node)
{
	
}

ng::scenegraph::CullingFlags ng::scenegraph::CullingWalker::isInView(RenderableNode * node, CameraNode * camera)
{
	//aabb 
	//	1		0
	//	|	2	|	3
	//	|	|	|	|
	//	5	|	4	|
	//		6		7
	CullingFlags ret;
	__mmask8 isFullyInside = 0b11111111;
	__mmask8 isFullyOutside = 0b00000000;

	ng::bvolumes::AABB* aabb = &node->m_AABB;

	ng::math::Vec3f p0, p1, p2, p3, p4, p5, p6, p7;

	float a, b, c, d;

	ALIGN(16) float distances[8];
	
	p0 = aabb->getPoint0();
	p1 = aabb->getPoint1();
	p2 = aabb->getPoint2();
	p3 = aabb->getPoint3();
	p4 = aabb->getPoint4();
	p5 = aabb->getPoint5();
	p6 = aabb->getPoint6();
	p7 = aabb->getPoint7();

	__m256 X = _mm256_set_ps(p0.x, p1.x, p2.x, p3.x, p4.x, p5.x, p6.x, p7.x);
	__m256 Y = _mm256_set_ps(p0.y, p1.y, p2.y, p3.y, p4.y, p5.y, p6.y, p7.y);
	__m256 Z = _mm256_set_ps(p0.z, p1.z, p2.z, p3.z, p4.z, p5.z, p6.z, p7.z);

	__m256 dists;

	//check all points from left-plane
	{
		a = camera->leftPlane.a;
		b = camera->leftPlane.b;
		c = camera->leftPlane.c;
		d = camera->leftPlane.d;

		dists = _mm256_add_ps(
			_mm256_add_ps(
				_mm256_mul_ps(_mm256_set1_ps(a), X),
				_mm256_mul_ps(_mm256_set1_ps(b), Y)
			),
			_mm256_add_ps(
				_mm256_mul_ps(_mm256_set1_ps(c), Z),
				_mm256_set1_ps(d)
			)
		);
		
		if (_mm256_cmp_ps_mask(dists, _mm256_set1_ps(0.0f), _CMP_LT_OS) != 0b11111111) {
			isFullyInside = 0b00000000;
		}

		isFullyOutside |= _mm256_cmp_ps_mask(dists, _mm256_set1_ps(0.0f), _CMP_GE_OS);

	}

	//check all points from right-plane
	{
		a = camera->rightPlane.a;
		b = camera->rightPlane.b;
		c = camera->rightPlane.c;
		d = camera->rightPlane.d;

		dists = _mm256_add_ps(
			_mm256_add_ps(
				_mm256_mul_ps(_mm256_set1_ps(a), X),
				_mm256_mul_ps(_mm256_set1_ps(b), Y)
			),
			_mm256_add_ps(
				_mm256_mul_ps(_mm256_set1_ps(c), Z),
				_mm256_set1_ps(d)
			)
		);

		if (_mm256_cmp_ps_mask(dists, _mm256_set1_ps(0.0f), _CMP_LT_OS) != 0b11111111) {
			isFullyInside = 0b00000000;
		}

		isFullyOutside |= _mm256_cmp_ps_mask(dists, _mm256_set1_ps(0.0f), _CMP_GE_OS);
	}

	//check all points from top-plane
	{
		a = camera->topPlane.a;
		b = camera->topPlane.b;
		c = camera->topPlane.c;
		d = camera->topPlane.d;

		dists = _mm256_add_ps(
			_mm256_add_ps(
				_mm256_mul_ps(_mm256_set1_ps(a), X),
				_mm256_mul_ps(_mm256_set1_ps(b), Y)
			),
			_mm256_add_ps(
				_mm256_mul_ps(_mm256_set1_ps(c), Z),
				_mm256_set1_ps(d)
			)
		);

		if (_mm256_cmp_ps_mask(dists, _mm256_set1_ps(0.0f), _CMP_LT_OS) != 0b11111111) {
			isFullyInside = 0b00000000;
		}

		isFullyOutside |= _mm256_cmp_ps_mask(dists, _mm256_set1_ps(0.0f), _CMP_GE_OS);
	}

	//check all points from bottom-plane
	{
		a = camera->bottomPlane.a;
		b = camera->bottomPlane.b;
		c = camera->bottomPlane.c;
		d = camera->bottomPlane.d;

		dists = _mm256_add_ps(
			_mm256_add_ps(
				_mm256_mul_ps(_mm256_set1_ps(a), X),
				_mm256_mul_ps(_mm256_set1_ps(b), Y)
			),
			_mm256_add_ps(
				_mm256_mul_ps(_mm256_set1_ps(c), Z),
				_mm256_set1_ps(d)
			)
		);

		if (_mm256_cmp_ps_mask(dists, _mm256_set1_ps(0.0f), _CMP_LT_OS) != 0b11111111) {
			isFullyInside = 0b00000000;
		}

		isFullyOutside |= _mm256_cmp_ps_mask(dists, _mm256_set1_ps(0.0f), _CMP_GE_OS);
	}

	//check all points from far-plane
	{
		a = camera->farPlane.a;
		b = camera->farPlane.b;
		c = camera->farPlane.c;
		d = camera->farPlane.d;

		dists = _mm256_add_ps(
			_mm256_add_ps(
				_mm256_mul_ps(_mm256_set1_ps(a), X),
				_mm256_mul_ps(_mm256_set1_ps(b), Y)
			),
			_mm256_add_ps(
				_mm256_mul_ps(_mm256_set1_ps(c), Z),
				_mm256_set1_ps(d)
			)
		);

		if (_mm256_cmp_ps_mask(dists, _mm256_set1_ps(0.0f), _CMP_LT_OS) != 0b11111111) {
			isFullyInside = 0b00000000;
		}

		isFullyOutside |= _mm256_cmp_ps_mask(dists, _mm256_set1_ps(0.0f), _CMP_GE_OS);
	}

	//check all points from near-plane
	{
		a = camera->nearPlane.a;
		b = camera->nearPlane.b;
		c = camera->nearPlane.c;
		d = camera->nearPlane.d;

		dists = _mm256_add_ps(
			_mm256_add_ps(
				_mm256_mul_ps(_mm256_set1_ps(a), X),
				_mm256_mul_ps(_mm256_set1_ps(b), Y)
			),
			_mm256_add_ps(
				_mm256_mul_ps(_mm256_set1_ps(c), Z),
				_mm256_set1_ps(d)
			)
		);

		if (_mm256_cmp_ps_mask(dists, _mm256_set1_ps(0.0f), _CMP_GT_OS) != 0b11111111) {
			isFullyInside = 0b00000000;
		}

		isFullyOutside |= _mm256_cmp_ps_mask(dists, _mm256_set1_ps(0.0f), _CMP_LE_OS);
	}

	//if it is fully inside
	if (isFullyInside == 0b11111111) {
		ret.setFlags(EVERYTHING_IN_FRUSTRUM);
		return ret;
	}
	//if it is fully outside
	if (isFullyOutside == 0b11111111) {
		ret.setFlags(NOTHING_IN_FRUSTRUM);
		return ret;
	}
	
	//see if bounding sphere is inside frustrum
	{
		float x = node->boundingSphere.center->x;
		float y = node->boundingSphere.center->y;
		float z = node->boundingSphere.center->z;

		__m256 A = _mm256_set_ps(
			camera->leftPlane.a,
			camera->rightPlane.a,
			camera->topPlane.a,
			camera->bottomPlane.a,
			camera->nearPlane.a,
			camera->farPlane.a,
			0.0f,
			0.0f
		);

		__m256 B = _mm256_set_ps(
			camera->leftPlane.b,
			camera->rightPlane.b,
			camera->topPlane.b,
			camera->bottomPlane.b,
			camera->nearPlane.b,
			camera->farPlane.b,
			0.0f,
			0.0f
		);

		__m256 C = _mm256_set_ps(
			camera->leftPlane.c,
			camera->rightPlane.c,
			camera->topPlane.c,
			camera->bottomPlane.c,
			camera->nearPlane.c,
			camera->farPlane.c,
			0.0f,
			0.0f
		);


		__m256 numerator = _mm256_andnot_ps(_mm256_castsi256_ps(_mm256_set1_epi32(0x80000000)) , //signmask
			_mm256_add_ps(
				_mm256_add_ps(
					_mm256_mul_ps(
						A,
						_mm256_set1_ps(x)
					),
					_mm256_mul_ps(
						B,
						_mm256_set1_ps(y)
					)
				),
				_mm256_add_ps(
					_mm256_mul_ps(
						C,
						_mm256_set1_ps(x)
					),
					_mm256_set_ps(
						camera->leftPlane.d,
						camera->rightPlane.d,
						camera->topPlane.d,
						camera->bottomPlane.d,
						camera->nearPlane.d,
						camera->farPlane.d,
						0.0f,
						0.0f
					)
				)
			)
		);

		__m256 denominator = _mm256_rsqrt_ps(
			_mm256_add_ps(
				_mm256_add_ps(
					_mm256_mul_ps(A, A),
					_mm256_mul_ps(B, B)
				),
				_mm256_mul_ps(C, C)
			)
		);

		dists = _mm256_div_ps(numerator, denominator);

		/*last two in dists will álways be zero and will therefore automatically be less than radius. 
		Therefore if comparison is greatar than 0b00000011 some part of the model must be inside frustrum */
		if (_mm256_cmp_ps_mask(dists, _mm256_set1_ps(node->boundingSphere.radius), _CMP_LT_OS) > 0b00000011) {
			ret.setFlags(MODEL_IN_FRUSTRUM | AABB_IN_FRUSTRUM);
			return ret;
		}

	}

}

ng::scenegraph::CullingWalker::CullingWalker(Scene * scene)
{
	m_Scene = scene;



}

void ng::scenegraph::CullingWalker::walk(CameraNode * camera)
{

	for (auto it = m_Scene->m_SceneRenderState.begin(); it != m_Scene->m_SceneRenderState.end(); ++it) {
		for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
			std::vector<RenderableNode*>* rns = &it2->second;
			for (int i = 0; i < rns->size(); ++i) {
				CullingFlags cf = isInView((*rns)[i], camera);
				//nothing
				if (cf.nothingInFrustrum()) {

				} //all
				else if (cf.nothingInFrustrum()) {

				} //
				else if (cf.modelInFrustrum()) {

				}
				else {

				}
			}
		}
	}

}


