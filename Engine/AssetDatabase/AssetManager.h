#pragma once
#include <queue>

#include "../EventSystem/Event.h"
#include "AssetLoader.h"

namespace gns::assets
{
class AssetManager
{
public:
	
	struct AssetLoadedEvent
	{
		gns::guid loadedAsset;
		AssetType assetType;
		std::string assetName;
		std::vector<gns::guid> primaryObjects {};
		std::vector<gns::guid> secondaryObjects {};
	};

	struct AssetLoadFailedEvent
	{
		gns::guid assetGuid;
		std::string assetName;
		std::string message;
	};
	
	GNS_API static gns::Event_T<AssetLoadedEvent> OnAssetLoadedEvent;
	GNS_API static gns::Event_T<AssetLoadFailedEvent> OnAssetLoadFailedEvent;
	GNS_API static std::queue<AssetLoadedEvent> AssetLoadedEventQueue;
	GNS_API static std::queue<AssetLoadFailedEvent> AssetLoadFailedEventQueue;
	GNS_API static void LoadAsset(AssetInfo info);

};
}
