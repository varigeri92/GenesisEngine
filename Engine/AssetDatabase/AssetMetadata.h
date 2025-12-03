#pragma once
#include "../Object/Guid.h"

namespace gns
{
	struct RuntimeAsset{
		size_t assetId = static_cast<size_t>(-1);
		std::string path = {};
	};

	class AssetRegistry
	{
	private:
		GNS_API static RuntimeAsset sInvalidAssetEntry;
		GNS_API static std::unordered_map<guid, RuntimeAsset> sRegistry;
	public:
		static const RuntimeAsset& Get(guid guid)
		{
			if(sRegistry.contains(guid))
				return sRegistry[guid];
			return sInvalidAssetEntry;
		}

		static void Add(guid guid, RuntimeAsset asset)
		{
			if(sRegistry.contains(guid))
			{
				LOG_INFO("Guid Collision... overvrite entry.");
			}
			sRegistry[guid] = std::move(asset);
		}

		static void ListAssets()
		{
			for (auto& it : sRegistry) {
				// Do stuff
				LOG_INFO(it.second.path);
			}
		}
	};

	struct Material
	{
		guid asset_guid;
		std::string asset_name;
		std::string vert_shader_path;
		std::string frag_shader_path;
	};

	struct AssetSubmesh
	{
		uint32_t mesh_index;
		guid mesh_guid;
	};

	struct MeshAsset
	{
		guid asset_guid;
		std::string asset_name;
		std::string src_path;
		std::vector<AssetSubmesh> sub_meshes;
	};
	struct AssetMetadata
	{
		guid assetGuid;
		std::string assetName;
		std::string srcPath;
		assetLibrary::AssetType assetType;

		std::unordered_map<guid, AssetMetadata> sub_assets;
	};
}
