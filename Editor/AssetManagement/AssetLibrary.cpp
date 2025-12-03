#include "AssetLibrary.h"
#include <filesystem>
#include "yaml-cpp/yaml.h"

#include "AssetImporter.h"
#include "../PathManager.h"
#include "../../Engine/Utils/FileSystemUtils.h"

namespace fs = std::filesystem;
std::unordered_map<gns::guid, gns::AssetMetadata> gns::editor::assets::AssetLibrary::assetDatabase = {};

bool gns::editor::assets::AssetLibrary::TryGetAsset(guid guid, AssetMetadata& outMeta)
{
	if(assetDatabase.contains(guid))
	{
		outMeta = assetDatabase[guid];
		return true;
	}
	return false;
}

void gns::editor::assets::AssetLibrary::ScanAssetLibrary()
{
	LOG_INFO("Project path: " + PathManager::ProjectPath);
	LOG_INFO("Scanning assetLibrary ... ");
	std::vector<std::string> mark_for_delete = {};
	for (const auto& entry : fs::directory_iterator(PathManager::AssetDatabasePath))
	{
		YAML::Node databaseNode = YAML::LoadFile(entry.path().string());
		guid guid = databaseNode["guid"].as<uint64_t>();
		std::string meta_path = databaseNode["meta_path"].as<std::string>();
		if(!fileUtils::FileExists(PathManager::FromAssetsRelative(meta_path)))
		{
			mark_for_delete.push_back(entry.path().string());
			continue;
		}

		YAML::Node metaNode = YAML::LoadFile(PathManager::FromAssetsRelative(meta_path));
		assetDatabase[guid] =
		{
			guid,
			metaNode["asset_name"].as<std::string>(),
			metaNode["src_path"].as<std::string>(),
			static_cast<assetLibrary::AssetType>(metaNode["asset_type"].as<uint32_t>())
		};
		AssetRegistry::Add(guid, { .assetId = guid, .path = PathManager::AssetsPath + assetDatabase[guid].srcPath });
	}
	LOG_INFO("Scan completed. \n \t - Found " + std::to_string(assetDatabase.size()) + " assets. \n \t - Marked " 
		+ std::to_string(mark_for_delete.size()) + " for delete (no Meta files where found!)");
	for (const auto & path : mark_for_delete)
	{
		fileUtils::DeleteFile(path);
	}
	
}
