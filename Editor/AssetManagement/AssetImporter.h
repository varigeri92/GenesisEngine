#pragma once
#include "Genesis.h"
#include "AssetMetadata.h"
 

namespace gns::editor::assets
{

	struct IAssetImportOptions
	{
		
	};
	struct MeshImportOptions : public IAssetImportOptions
	{
		bool isStatic = true;
		bool import_materials = true;
		bool import_skeleton = true;
		bool import_textures = true;
	};

	struct TextureImportOptions : public IAssetImportOptions
	{
		bool hdr = false;
		gns::assets::TextureAssetType textureAssetType;
	};

	class AssetImporter
	{
	public:
		static bool ImportAsset(const std::string& filePath, bool reImport, std::function<void()> callback);
		static bool IsImported(const std::string& filePath);
		static bool IsMeta(const std::string& filePath);

		static AssetMetadata* GetMetadata(const std::string& assetPath);
		static gns::assets::MeshAssetDescription GetMeshAsset(const AssetMetadata& asset_metadata);

		static gns::assets::AssetType GetAssetType(const std::string& extension);

	private:
		static bool ImportMesh(std::string file_path, MeshImportOptions options, guid guid);
		static bool ImportTexture(const std::string& file_path, TextureImportOptions& out_options, guid guid);
		static bool ImportAssetInternal(
			const gns::assets::AssetType assetType, const std::string& relative_path, 
			const gns::guid guid, void* options);
	};
}
