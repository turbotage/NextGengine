#pragma once
#include <vector>
#include <unordered_map>

#include "../Entities/model.h"
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

			std::unordered_multimap<GridIndex, ng::entity::Model*> m_Collidables;

		public:

			GridIndex getIndex(ng::math::Vec3 position);
			ng::math::Vec3 getCenterPosition(const GridIndex* index);

			

			UniformGrid();
			UniformGrid(ng::math::Vec3s worldSizeMultiplier, float gridSize, uint16 expectedNumOfObjects);
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