#pragma once
#include "../Object/Guid.h"

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
	struct AssetMetadata
	{
		guid assetGuid;
		std::string assetName;
		std::string srcPath;
		assetLibrary::AssetType assetType;

		std::unordered_map<guid, AssetMetadata> sub_assets;
	};
}
