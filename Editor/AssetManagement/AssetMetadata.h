#pragma once
#include "Genesis.h"

namespace gns::assets
{
	enum class AssetType;
}

namespace gns
{
	struct AssetMetadata
	{
		gns::guid assetGuid;
		std::string assetName;
		std::string srcPath;
		gns::assets::AssetType assetType;

		std::unordered_map<gns::guid, AssetMetadata> sub_assets;
	};
}
