#pragma once
#include <API.h>
#include <string>
#include <vector>
#include "../Object/Guid.h"
#include "functional"
namespace gns
{
	struct MeshAsset;
}

namespace gns::rendering
{
	struct Texture;
	class Device;
}

namespace gns::assetLibrary
{
	enum class AssetType
	{
		None, Mesh, Texture, Sound, Material, Shader, Compute
	};

	GNS_API void LoadAsset(const std::string& filePath);
	void LoadTexture(const std::string& filePath, rendering::Texture& texture);
	GNS_API void LoadMeshAsset(const MeshAsset& mesh_asset, 
		const std::function<void(const std::vector<guid>&, const std::vector<guid>&)>& onLoadSuccess_callback);
}

