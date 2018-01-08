#pragma once
#include <vector>
#include <unordered_map>

#include "../Entities/model.h"
#include "../Math/Vec3f.h"
#include "../Math/hash_functions.h"
#include "../Math/Vec3fs.h"
#include "../camera.h"


namespace ng {
	namespace dstructs {

		typedef ng::math::Vec3fs GridIndex;

		class UniformGrid
		{
		private:

			float m_GridSize;
			float m_GridIndexRadius;

			ng::math::Vec3fs m_WorldSizeMultipler;

			std::unordered_multimap<GridIndex, ng::entity::Model*> m_Collidables;

		public:

			GridIndex getIndex(ng::math::Vec3f position);
			ng::math::Vec3f getCenterPosition(const GridIndex* index);

			

			UniformGrid();
			UniformGrid(ng::math::Vec3fs worldSizeMultiplier, float gridSize, uint16 expectedNumOfObjects);
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