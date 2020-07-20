#include "ng_utility.h"



std::string ng::getShaderDirectoryPath(std::string projectName)
{
	return ng::getAssetsDirectoryPath(projectName) + "\\shaders\\";
}

std::string ng::getAssetsDirectoryPath(std::string projectName)
{
	namespace fs = std::filesystem;
	auto execPath = fs::current_path();
	auto sExecPath = execPath.string();
	auto pos = sExecPath.find(projectName);
	auto subPath = sExecPath.substr(0, pos + projectName.length());
	subPath += "\\Assets\\";
	return subPath;
}


typedef std::multimap<std::string, std::pair<std::string, int>> ShaderMultimap;

constexpr int spacing = 5;

void recurseShaderPath(std::string& ret, ShaderMultimap& map, ShaderMultimap::iterator mapIt, bool printParent) {
	
	if (printParent) {
		ret += std::string(spacing * (mapIt->second.second), ' ');
		ret += mapIt->first + "\n";
	}

	if (map.find(mapIt->second.first) == map.end()) {
		ret += std::string(spacing * (mapIt->second.second + 1), ' ');
		ret += mapIt->second.first + '\n';
		return;
	}

	auto range = map.equal_range(mapIt->second.first);
	recurseShaderPath(ret, map, range.first, true);
	for (auto itr = ++range.first; itr != range.second; ++itr) {
		recurseShaderPath(ret, map, itr, false);
	}

}

std::string ng::listShaders(std::string projectName)
{
	std::string ret;
	namespace fs = std::filesystem;
	auto shaderPathString = getShaderDirectoryPath(projectName);
	fs::path shaderPath = shaderPathString;
	// parentName, (name,depth)
	ShaderMultimap pathStrings;

	

	// add strings to map
	for (auto i = fs::recursive_directory_iterator(shaderPathString); i != fs::recursive_directory_iterator(); ++i) {
		int depth = i.depth() + 1;
		std::string pathString = i->path().filename().string();
		std::string parentPathString = i->path().parent_path().filename().string();

		pathStrings.emplace(parentPathString, std::make_pair(pathString, depth));
	}
	ret += std::string(spacing, ' ') + "shaders\n";
	auto range = pathStrings.equal_range("shaders");
	for (auto it = range.first; it != range.second; ++it) {
		recurseShaderPath(ret, pathStrings, it, false);
	}
	
	return ret;
}








std::string ng::getFileExtension(std::string filename)
{
	size_t pos = filename.find_last_of(".");
	if (pos == std::string::npos) {
		return "";
	}
	return filename.substr(pos + 1);
}









std::vector<uint8> ng::loadFile(const std::string& filename)
{
	std::ifstream is(filename, std::ios::binary | std::ios::ate);
	std::vector<uint8_t> bytes;
	if (!is.fail()) {
		size_t size = is.tellg();
		is.seekg(0);
		bytes.resize(size);
		is.read((char*)bytes.data(), size);
	}
	return bytes;
}

void ng::writeFile(std::vector<uint8> bytes, const std::string& filename)
{
	std::ofstream file;
	file.open(filename);
	file.write((char*)bytes.data(), bytes.size());
	file.close();
}

std::vector<uint8> ng::loadNGFile(const std::string& filename)
{
	return loadFile(filename);
}


































/*
std::vector<float> ng::getFloatsFromBytes(std::vector<uint8> bytes)
{
	std::vector<float> ret;
#ifndef NDEBUG
	if (!(bytes.size() % 4)) {
		std::runtime_error("Bytes was not a multibule of 4, cannot convert to floats");
	}
#endif
	ret.resize(bytes.size() / 4);
	memcpy(ret.data(), bytes.data(), bytes.size());
	return ret;
}

std::vector<ng::Vertex2D> ng::getVertex2DsFromBytes(std::vector<uint8> bytes)
{
	std::vector<ng::Vertex2D> ret;
#ifndef NDEBUG
	if (!(bytes.size() % 4)) {
		std::runtime_error("Bytes was not a multibule of 4, cannot convert to floats");
	}
	if (!((bytes.size() / 4) % 2)) {
		std::runtime_error("Bytes was not a multiple of 8, cannot convert to Vertex2D");
	}
#endif
	ret.resize(bytes.size() / 8);
	memcpy(ret.data(), bytes.data(), bytes.size());
	return ret;
}

std::vector<ng::Vertex2D_3C> ng::getVertex2D_3CsFromBytes(std::vector<uint8> bytes)
{
	std::vector<ng::Vertex2D_3C> ret;
#ifndef NDEBUG
	if (!(bytes.size() % 4)) {
		std::runtime_error("Bytes was not a multibule of 4, cannot convert to floats");
	}
	if (!((bytes.size() / 4) % 5)) {
		std::runtime_error("Bytes was not a multiple of 20, cannot convert to Vertex2D");
	}
#endif
	ret.resize(bytes.size() / 20);
	memcpy(ret.data(), bytes.data(), bytes.size());
	return ret;
}

std::vector<ng::Vertex3D> ng::getVertex3DsFromBytes(std::vector<uint8> bytes)
{
	std::vector<ng::Vertex3D> ret;
#ifndef NDEBUG
	if (!(bytes.size() % 4)) {
		std::runtime_error("Bytes was not a multibule of 4, cannot convert to floats");
	}
	if (!((bytes.size() / 4) % 3)) {
		std::runtime_error("Bytes was not a multiple of 12, cannot convert to Vertex3D");
	}
#endif
	ret.resize(bytes.size() / 12);
	memcpy(ret.data(), bytes.data(), bytes.size());
	return ret;
}








std::vector<uint8> ng::getBytesFromFloats(std::vector<float> floats)
{
	std::vector<uint8> bytes(floats.size() * 4);
	memcpy(bytes.data(), floats.data(), bytes.size());
	return bytes;
}

std::vector<uint8> ng::getBytesFromVertex2Ds(std::vector<Vertex2D> vertices)
{
	std::vector<uint8> bytes(vertices.size() * 8);
	memcpy(bytes.data(), vertices.data(), bytes.size());
	return bytes;
}

std::vector<uint8> ng::getBytesFromVertex2D_3Cs(std::vector<Vertex2D_3C> vertices)
{
	std::vector<uint8> bytes(vertices.size() * 20);
	memcpy(bytes.data(), vertices.data(), bytes.size());
	return bytes;
}

std::vector<uint8> ng::getBytesFromVertex3Ds(std::vector<Vertex3D> vertices)
{
	std::vector<uint8> bytes(vertices.size() * 12);
	memcpy(bytes.data(), vertices.data(), bytes.size());
	return bytes;
}

*/



