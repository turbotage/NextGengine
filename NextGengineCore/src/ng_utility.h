#pragma once

#include "pch.h"

// Helver functions
namespace ng {

	struct Vertex2D { glm::vec2 vertex; };
	struct Vertex2D_3C { glm::vec2 vertex; glm::vec3 color; };
	struct Vertex3D { glm::vec3 vertex; };

	struct Uniform2D { glm::mat4 model; };


	// Returns a string to the shader directory
	std::string getShaderDirectoryPath(std::string projectName = "NextGengine");
	// Returns a string to the assets directory
	std::string getAssetsDirectoryPath(std::string projectName = "NextGengine");
	// Returns a list o fthe directories and the shaders in the directories
	std::string listShaders(std::string projectName = "NextGengine");
	// Return the string of the extension, (the stuff after the dot)
	std::string getFileExtension(std::string filename);
	// Load a file
	std::vector<uint8> loadFile(const std::string& filename);
	// Write to file
	void writeFile(std::vector<uint8> bytes, const std::string& filename);
	// Load NG file, (virtual filing) // Not yet implemented
	std::vector<uint8> loadNGFile(const std::string& filename);


	template<class T>
	void getFromBytes(std::vector<uint8>& src, std::vector<T>& dst) {
		dst.resize(src.size() / sizeof(T));
		memcpy(dst.data(), src.data(), src.size());
	}

	template<class T>
	void setFromType(std::vector<T>& src, std::vector<uint8>& dst) {
		dst.resize(src.size() * sizeof(T));
		memcpy(dst.data(), src.data(), src.size() * sizeof(T));
	}

	// STUPID
	// Get floats from bytes
	std::vector<float> getFloatsFromBytes(std::vector<uint8> bytes);
	// Get Vertex2D from bytes
	std::vector<Vertex2D> getVertex2DsFromBytes(std::vector<uint8> bytes);
	// Get Vertex2D_3C from bytes
	std::vector<Vertex2D_3C> getVertex2D_3CsFromBytes(std::vector<uint8> bytes);
	// Get Vertex3D from bytes
	std::vector<Vertex3D> getVertex3DsFromBytes(std::vector<uint8> bytes);
	// Get bytes from floats
	std::vector<uint8> getBytesFromFloats(std::vector<float> floats);
	// Get bytes from Vertex2D
	std::vector<uint8> getBytesFromVertex2Ds(std::vector<Vertex2D> vertices);
	// Get bytes from Vertex2D_3C
	std::vector<uint8> getBytesFromVertex2D_3Cs(std::vector<Vertex2D_3C> vertices);
	// Get bytes from Vertex3D
	std::vector<uint8> getBytesFromVertex3Ds(std::vector<Vertex3D> vertices);

}

