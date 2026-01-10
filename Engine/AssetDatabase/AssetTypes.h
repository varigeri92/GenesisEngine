#pragma once
namespace gns::assets
{
	enum class AssetType { None, Mesh, Texture, Sound, Material, Shader, Compute };
	enum class AssetKind { Invalid, Source, Baked };
	enum class TextureAssetType { Texture2D, Array, CubeMap, Sprite };
}