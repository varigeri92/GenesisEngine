#include "gnspch.h"
#include "PathHelper.h"

std::string gns::PathHelper::AssetsPath = "";
std::string gns::PathHelper::ResourcesPath = "";

std::string gns::PathHelper::FromAssetsRelative(const std::string& relative_path)
{
	return AssetsPath + relative_path;
}

std::string gns::PathHelper::FromResourcesRelative(const std::string& relative_path)
{
	return ResourcesPath + relative_path;
}
