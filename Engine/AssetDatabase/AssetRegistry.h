#pragma once
#include "../Object/Guid.h"

namespace gns::assets
{
	enum class AssetKind { Invalid, Source, Baked };
	struct AssetInfo
	{
		AssetKind assetKind{ AssetKind::Invalid };
		guid assetGuid;
		std::string filePath{};
		size_t offset{0};
		size_t size{0};
	};
	class AssetRegistry
	{
	private:
		GNS_API static AssetInfo sInvalidAssetEntry;
		GNS_API static std::unordered_map<guid, AssetInfo> sRegistry;
	public:
		static const AssetInfo& Get(guid guid)
		{
			if(sRegistry.contains(guid))
				return sRegistry[guid];

			return sInvalidAssetEntry;
		}

		static void Add(guid guid, AssetInfo asset)
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
				LOG_INFO(it.second.filePath);
			}
		}
	};

}

namespace gns
{
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
}