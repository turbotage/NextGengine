#pragma once

#include <vector>
#include "../Entities\model.h"
#include "../BoundingVolumes\general_bv.h"
#include "../Entities/object.h"


#define INVALID 0x00
#define EMPTY 0x01
#define INTERMEDIATE 0x02

namespace ng {
	namespace dstructs {

		class OcTree
		{
		private:
			
			class Node {
			private:
				Node* parentNode;
				Node* children[8];
			public:
				std::vector<uint8> nodeIndex;
				std::vector<ng::entity::Object> objects;
				//LeafNode();
			};

			Node* startNode;

		public:

			//std::vector<Node*> getEnclosedIndices(ng::entity::Object* object);

			//std::vector<ng::entity::Object>* getObjects(std::vector<uint8> nodeIndex);



		};

	}
}


