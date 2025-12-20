#include "gnspch.h"
#include "AssetManager.h"
gns::Event_T<gns::assets::AssetManager::AssetLoadedEvent> gns::assets::AssetManager::OnAssetLoadedEvent = {};
gns::Event_T<gns::assets::AssetManager::AssetLoadFailedEvent> gns::assets::AssetManager::OnAssetLoadFailedEvent = {};
std::vector<gns::assets::AssetManager::AssetLoadedEvent> gns::assets::AssetManager::AssetLoadedEventQueue = {};
std::vector<gns::assets::AssetManager::AssetLoadFailedEvent> gns::assets::AssetManager::AssetLoadFailedEventQueue ={};

void gns::assets::AssetManager::LoadAsset(AssetInfo info)
{
	AssetLoader loader{ info };
	loader.LoadAsset();
	for (const auto & assetLoadedEvent : AssetLoadedEventQueue)
	{
		OnAssetLoadedEvent.Dispatch(assetLoadedEvent);
		
	}

	for (const auto& loadFailedEvent : AssetLoadFailedEventQueue)
	{
		OnAssetLoadFailedEvent.Dispatch(loadFailedEvent);
	}
}
