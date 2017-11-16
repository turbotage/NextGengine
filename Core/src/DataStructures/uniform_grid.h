#pragma once

#include "../Math/vec3.h"
#include "../Entities/object.h"
#include <unordered_map>

namespace ng {
	namespace dstructs {

		struct GridIndex {
			short xIndex;
			short yIndex;
			short zIndex;
		};

		namespace std {
			template<> struct hash<GridIndex> {
				size_t operator()(GridIndex const& index) const {
					
				}
			};
		}

		class UniformGrid
		{
		private:

			float m_GridSize;
			int m_WorldSizeX;
			int m_WorldSizeY;
			int m_WordSizeZ;

			std::unordered_map<GridIndex, std::vector<ng::entity::Object>> m_GridBlocks;


		public:



			GridIndex getIndex(ng::math::Vec3 position);



			UniformGrid();
			~UniformGrid();
		};
	}
}

