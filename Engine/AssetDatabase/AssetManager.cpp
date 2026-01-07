#include "gnspch.h"
#include "AssetManager.h"

#include "../Object/Object.h"
gns::Event_T<gns::assets::AssetManager::AssetLoadedEvent> gns::assets::AssetManager::OnAssetLoadedEvent = {};
gns::Event_T<gns::assets::AssetManager::AssetLoadFailedEvent> gns::assets::AssetManager::OnAssetLoadFailedEvent = {};
std::queue<gns::assets::AssetManager::AssetLoadedEvent> gns::assets::AssetManager::AssetLoadedEventQueue = {};
std::queue<gns::assets::AssetManager::AssetLoadFailedEvent> gns::assets::AssetManager::AssetLoadFailedEventQueue ={};

void gns::assets::AssetManager::LoadAsset(AssetInfo info)
{
	AssetLoader loader{ info };
	loader.LoadAsset();
	while (AssetLoadedEventQueue.size() > 0)
	{
		auto ale = AssetLoadedEventQueue.front();
		OnAssetLoadedEvent.Dispatch(ale);
		AssetLoadedEventQueue.pop();
	}

	while (AssetLoadFailedEventQueue.size() > 0)
	{
		auto ale = AssetLoadFailedEventQueue.front();
		OnAssetLoadFailedEvent.Dispatch(ale);
		AssetLoadFailedEventQueue.pop();
	}
}
