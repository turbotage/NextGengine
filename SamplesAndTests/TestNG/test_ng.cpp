
#include "../NextGengine/NextGengine.h"

#include <iostream>

int main() {

	std::cout << ng::getShaderDirectoryPath("NextGengine") << std::endl;

	const std::vector<ng::Vertex2D> vertices = {
		{{-0.5f, -0.5f}},
		{{0.5f, 0.5f}},
		{{0.5f, -0.5f}},
		{{-0.5f, -0.5f}},
		{{0.5f, 0.5f}},
		{{-0.5f, 0.5f}}
	};

	auto bytes = ng::getBytesFromVertex2Ds(vertices);

	auto assetPath = ng::getAssetsDirectoryPath("NextGengine");

	ng::writeFile(bytes, assetPath + "test_assets/test_vert2D.v2d");

	auto bytes2 = ng::loadFile(assetPath + "test_assets/test_vert2D.v2d");

	auto vertecies2 = ng::getVertex2DsFromBytes(bytes2);

	std::cout << ng::getAssetsDirectoryPath("NextGengine") << std::endl;

}

