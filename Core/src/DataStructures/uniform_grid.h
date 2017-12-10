#pragma once
#include <vector>
#include <unordered_map>

#include "../Entities/object.h"
#include "../Math/vec3.h"
#include "../Math/hash_functions.h"
#include "../Math/vec3s.h"
#include "../camera.h"


namespace ng {
	namespace dstructs {

		typedef ng::math::Vec3s GridIndex;

		class UniformGrid
		{
		private:
			
			float m_GridSize;
			float m_GridIndexRadius;
			ng::math::Vec3s m_WorldSizeMultipler;

			std::unordered_multimap<GridIndex, Collidable*> m_Collidables;
			std::unordered_multimap<GridIndex, Renderable*> m_Renderables;

		public:

			GridIndex getIndex(ng::math::Vec3 position);
			ng::math::Vec3 getCenterPosition(const GridIndex* index);

			std::vector<Renderable*> frustrumCull(Camera& cam);

			

			UniformGrid();
			UniformGrid(uint16 expectedNumOfObjects, float gridSize, ng::math::Vec3s worldSizeMultiplier);
			~UniformGrid();
		};
	}
}

namespace std {
	template<> struct hash<ng::dstructs::GridIndex> {
		size_t operator()(ng::dstructs::GridIndex const& index) const {
			std::vector<short> v{ index.x, index.y, index.z };
			return ng::math::hashCombineMany(&v);
		}
	};
}