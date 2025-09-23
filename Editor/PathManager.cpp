#include "PathManager.h"

std::string PathManager::ProjectPath = {};
std::string PathManager::AssetDatabasePath ={};
std::string PathManager::AssetsPath = {};
std::string PathManager::ResourcesPath = {};

std::string PathManager::FromProjectRelative(const std::string& relativePath)
{
	return ProjectPath + relativePath;
}

std::string PathManager::FromAssetsRelative(const std::string& relativePath)
{
	return AssetsPath + relativePath;
}

std::string PathManager::FromResourcesRelative(const std::string& relativePath)
{
	return ResourcesPath + relativePath;
}

std::string PathManager::FromDatabaseRelative(const std::string& relativePath)
{
	return AssetDatabasePath + relativePath;
}
