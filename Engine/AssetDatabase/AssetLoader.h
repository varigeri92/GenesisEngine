#pragma once
#include <API.h>
#include <string>
#include <vector>
#include <functional>
#include "../Object/Guid.h"
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
		GNS_API static bool IsTextureHDR(const std::string& texture_path);
		GNS_API static bool ReadTextureData(const std::string& texture_path, 
			uint32_t& width, uint32_t& height, int32_t& channels);
	private:
		const AssetInfo& assetInfo;
		bool LoadSourceAsset();
		bool LoadBakedAsset();

		bool LoadMeshSource(MeshAssetDescription mesh_asset);
		void LoadTextureSource(TextureAssetDescription texture_asset);

		MeshAssetDescription GetMeshAssetDescription(const std::string& assetFilePath);
		TextureAssetDescription GetTextureAssetDescription(const std::string& assetFilePath);
	};

}

