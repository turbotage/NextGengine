#include "ng_utility.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>

std::string ng::getShaderDirectoryPath(std::string projectName)
{
	namespace fs = std::filesystem;
	auto execPath = fs::current_path();
	auto sExecPath = execPath.string();
	auto pos = sExecPath.find(projectName);
	auto subPath = sExecPath.substr(0, pos + projectName.length());
	subPath += "\\NextGengine\\shaders\\";
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
	size_t pos = filename.find_last_of(".") + 1;
	if (pos == std::string::npos) {
		return "";
	}
	return filename.substr(pos);
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










