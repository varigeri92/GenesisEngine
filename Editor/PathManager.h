#pragma once
#include <string>

class PathManager
{
public:
	static std::string ProjectPath;
	static std::string AssetDatabasePath;
	static std::string AssetsPath;
	static std::string ResourcesPath;

	static std::string FromProjectRelative(const std::string& relativePath);
	static std::string FromAssetsRelative(const std::string& relativePath);
	static std::string FromResourcesRelative(const std::string& relativePath);
	static std::string FromDatabaseRelative(const std::string& relativePath);
};
