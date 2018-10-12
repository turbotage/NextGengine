#pragma once
#include <vector>
#include <unordered_map>

#include "../Entities/model.h"
#include "vec3f.h"
#include "vec3s.h"
#include "hash_functions.h"
#include "vec3f.h"
#include "../camera.h"


namespace ng {
	namespace dstructs {

		typedef ngm::Vec3s GridIndex;

		class UniformGrid
		{
		private:

			float GridSize;
			float GridIndexRadius;

			ngm::Vec3f WorldSizeMultipler;

			std::unordered_multimap<GridIndex, ng::entity::Model*> Collidables;

		public:

			GridIndex getIndex(ngm::Vec3f position);
			ngm::Vec3f getCenterPosition(const GridIndex* index);

			

			UniformGrid();
			UniformGrid(ngm::Vec3f worldSizeMultiplier, float gridSize, uint16 expectedNumOfObjects);
			~UniformGrid();
		};
	}
}

namespace std {
	template<> struct hash<ng::dstructs::GridIndex> {
		size_t operator()(ng::dstructs::GridIndex const& index) const {
			std::vector<short> v{ index.x, index.y, index.z };
			return ngm::hashCombineMany(&v);
		}
	};
}