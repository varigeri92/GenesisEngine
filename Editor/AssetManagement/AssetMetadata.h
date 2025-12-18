#pragma once
#include "Genesis.h"

namespace gns::assetLibrary
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
		gns::assetLibrary::AssetType assetType;

		std::unordered_map<gns::guid, AssetMetadata> sub_assets;
	};
}
