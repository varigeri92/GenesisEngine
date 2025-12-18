#pragma once
#include <unordered_map>
#include "Genesis.h"
#include "AssetMetadata.h"

namespace gns::editor::assets
{
	class AssetLibrary
	{
	public:
		static std::unordered_map<guid, AssetMetadata> assetDatabase;

		static bool TryGetAsset(guid guid, AssetMetadata& outMeta);

		static void ScanAssetLibrary();

	};
}

