#pragma once

#include <vector>
#include "Objects\model.h"

#define INVALID 0x00
#define EMPTY 0x01
#define INTERMEDIATE 0x02

class OcTree
{
private:
	
	class Node {
	private:

		

		typedef struct {
			unsigned char type;
			unsigned char level;
		} NodeData;

	public:
		Node();
	};

public:
	OcTree();
	~OcTree();

	Node getIndex();

};

