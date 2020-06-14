#include "ng_utility.h"

#include <filesystem>
#include <iostream>

std::string ng::getShaderDirectoryPath(std::string projectName)
{
	namespace fs = std::filesystem;
	auto execPath = fs::current_path();
	auto sExecPath = execPath.string();
	std::cout << sExecPath << std::endl;
	auto pos = sExecPath.find(projectName);
	auto subPath = sExecPath.substr(0, pos + projectName.length());
	subPath += "\\NextGengine\\shaders\\";
	std::cout << subPath << std::endl;
	return "";
}
