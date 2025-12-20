#pragma once
#include <API.h>
#include <string>
#include <vector>
#include "../Object/Guid.h"
#include "functional"
#include "AssetRegistry.h"
#include "../EventSystem/Event.h"

namespace gns::rendering
{
	struct Texture;
	class Device;
}

namespace gns::assets
{
	GNS_API void LoadAsset(const std::string& filePath);
	void LoadTexture(const std::string& filePath, rendering::Texture& texture, bool* hdr);
	GNS_API void LoadMeshAsset(const MeshAssetDescription& mesh_asset, 
		const std::function<void(const std::vector<guid>&, const std::vector<guid>&)>& onLoadSuccess_callback);

	class AssetLoader
	{
	public:
		GNS_API AssetLoader(const AssetInfo& info);
		GNS_API ~AssetLoader() = default;
		GNS_API const std::vector<guid> LoadAsset();

	private:
		const AssetInfo& assetInfo;
		bool LoadSourceAsset();
		bool LoadBakedAsset();

		bool LoadMeshSource(MeshAssetDescription mesh_asset);
		void LoadTextureSource();

		MeshAssetDescription GetMeshAssetDescription(const std::string& assetFilePath);
	};

}

