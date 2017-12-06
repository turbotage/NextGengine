#pragma once
#include <vector>
#include <unordered_map>

#include "../Entities/object.h"
#include "../Math/vec3.h"
#include "../Math/vec3s.h"
#include "../Math/hash_functions.h"


namespace ng {
	namespace dstructs {

		struct GridIndex {
			uint16 xIndex, yIndex, zIndex;
		};

		class UniformGrid
		{
		private:
			
			float m_GridSize;
			ng::math::Vec3s m_WorldSizeMultipler;

			std::unordered_multimap<GridIndex, Collidable*> m_Collidables;
			std::unordered_multimap<GridIndex, Renderable*> m_Renderables;

		public:

			GridIndex getIndex(ng::math::Vec3 position);

			UniformGrid();
			UniformGrid(uint16 expectedNumOfObjects);
			~UniformGrid();
		};
	}
}

namespace std {
	template<> struct hash<ng::dstructs::GridIndex> {
		size_t operator()(ng::dstructs::GridIndex const& index) const {
			std::vector<short> v{ index.xIndex, index.yIndex, index.zIndex };
			return ng::math::hashCombineMany(&v);
		}
	};
}