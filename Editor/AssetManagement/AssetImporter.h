#pragma once
#include "Genesis.h"

namespace gns::editor::assets
{

	struct IAssetImportOptions
	{
		
	};
	struct MeshImportOptions : public IAssetImportOptions
	{
		bool isStatic = true;
		bool import_materials = true;
	};

	class AssetImporter
	{
	public:
		static bool ImportAsset(const std::string& filePath, bool reImport = false);
		static bool IsImported(const std::string& filePath);
		static bool IsMeta(const std::string& filePath);

		static void OpenImportWindow(assetLibrary::AssetType type);
		static AssetMetadata* GetMetadata(const std::string& assetPath);
		static MeshAsset GetMeshAsset(const AssetMetadata& asset_metadata);

		static assetLibrary::AssetType GetAssetType(const std::string& extension);

	private:
		static bool ImportMesh(std::string file_path, MeshImportOptions options, guid guid);
	};
}
