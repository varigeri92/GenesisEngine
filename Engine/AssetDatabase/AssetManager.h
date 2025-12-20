#pragma once
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
		std::vector<void*> rawData;
		void* component_ptr;
	};
	struct AssetLoadFailedEvent
	{
		gns::guid assetGuid;
		std::string assetName;
		std::string message;
	};
	GNS_API static gns::Event_T<AssetLoadedEvent> OnAssetLoadedEvent;
	GNS_API static gns::Event_T<AssetLoadFailedEvent> OnAssetLoadFailedEvent;
	GNS_API static std::vector<AssetLoadedEvent> AssetLoadedEventQueue;
	GNS_API static std::vector<AssetLoadFailedEvent> AssetLoadFailedEventQueue;
	GNS_API static void LoadAsset(AssetInfo info);
};
}