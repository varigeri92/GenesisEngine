#include "AssetImporter.h"
#include "Genesis.h"
#include "GenesisRendering.h"
#include "GenesisFileSystem.h"

#include "AssetLibrary.h"
#include "../PathManager.h"
#define YAML_CPP_STATIC_DEFINE
#include <fstream>
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include "../../Engine/Rendering/GuiWindowDrawer.h"
#include "../Gui/Windows/AssetImporterWindow.h"
#include "yaml-cpp/yaml.h"

namespace gns::assets
{
std::unordered_map<std::string, AssetType> fileExtensionAssetTypeMap = {
    {"", AssetType::None},
    {"fbx", AssetType::Mesh},
    {"obj", AssetType::Mesh},
    {"gltf", AssetType::Mesh},
    {"glb", AssetType::Mesh},

    {"frag", AssetType::Shader},
    {"vert", AssetType::Shader},

    {"comp", AssetType::Compute},

    {"jpg", AssetType::Texture},
    {"jpeg", AssetType::Texture},
    {"png", AssetType::Texture},
    {"exr", AssetType::Texture},
    {"hdr", AssetType::Texture},

    {"mat",AssetType::Material}
};

}

bool gns::editor::assets::AssetImporter::ImportAsset(const std::string& filePath, bool reImport)
{
    std::string relative_path;
    if(fileUtils::IsRootedPath(filePath))
    {
        relative_path = fileUtils::ToRelative(filePath, PathManager::AssetsPath);
        LOG_INFO(relative_path);
    }

    if (IsMeta(relative_path)) {
        LOG_INFO("File is 'meta' or 'gnsMesh'");
    	return true;
    }
    guid guid = Guid::GetNewGuid();
    if(IsImported(relative_path))
    {
        if (reImport)
	        try
	        {
				guid = YAML::LoadFile(
                    PathManager::FromAssetsRelative(relative_path + ".meta"))["asset_guid"].as<size_t>();
	        }
	        catch (const std::exception& e)
	        {
                LOG_ERROR("Failed to read YAML '{}': {}", relative_path, e.what());
	        }
        else
            return true;
    }
    gns::assets::AssetType assetType = assets::AssetImporter::GetAssetType(fileUtils::GetFileExtension(relative_path));
    AssetImporterWindow* importer_window = reinterpret_cast<AssetImporterWindow*>(gns::GuiWindowDrawer::GetWindow("Import Asset"));
	
	if (importer_window)
	{
		AssetImporterWindow::MeshImportSettings settings = 
			{true, true, false, true, false};
        importer_window->OpenMeshImporterWindow(filePath, assetType, settings);
	}


    return ImportAssetInternal(assetType, relative_path, guid);
}

bool gns::editor::assets::AssetImporter::IsImported(const std::string& filePath)
{
    if(fileUtils::FileExists(PathManager::FromAssetsRelative(filePath + ".meta")))
    {
        return true;
    }
    return false;
}

bool gns::editor::assets::AssetImporter::IsMeta(const std::string& filePath)
{
    if (fileUtils::HasFileExtension(filePath, "meta"))
    {
        return true;
    }

    if (fileUtils::HasFileExtension(filePath, "gnsMesh"))
    {
        return true;
    }
    return false;
}


gns::AssetMetadata* gns::editor::assets::AssetImporter::GetMetadata(const std::string& assetPath)
{
    std::string path = assetPath;
    if (fileUtils::IsRootedPath(assetPath))
        path = fileUtils::ToRelative(assetPath, PathManager::AssetsPath);
    if (!IsMeta(assetPath))
    {
         path += ".meta";
    }
    else
    {
        if (fileUtils::HasFileExtension(assetPath, "gnsMesh"))
        {
            path = YAML::LoadFile(PathManager::FromAssetsRelative(path))["file_path"].as<std::string>() + ".meta";
        }
    }
    
    YAML::Node metaFile = YAML::LoadFile(PathManager::FromAssetsRelative(path));
    guid asset_guid = metaFile["asset_guid"] .as<size_t>();
    if(AssetLibrary::assetDatabase.contains(asset_guid))
    {
		return &AssetLibrary::assetDatabase[asset_guid];
    }else
    {
        AssetLibrary::assetDatabase[asset_guid] =
        {
            asset_guid,
            metaFile["asset_name"].as<std::string>(),
            metaFile["src_path"].as<std::string>(),
            static_cast<gns::assets::AssetType>(metaFile["asset_type"].as<uint32_t>())
        };
        return &AssetLibrary::assetDatabase[asset_guid];
    }
}

gns::assets::MeshAssetDescription gns::editor::assets::AssetImporter::GetMeshAsset(const AssetMetadata& asset_metadata)
{
    YAML::Node meshAssetFile = YAML::LoadFile(PathManager::FromAssetsRelative(asset_metadata.srcPath) + ".gnsMesh");
    gns::assets::MeshAssetDescription asset = {meshAssetFile["asset_guid"].as<uint64_t>(),
    meshAssetFile["asset_name"].as<std::string>(),
    meshAssetFile["file_path"].as<std::string>(), {}};

    for (const auto & mesh : meshAssetFile["sub_meshes"])
    {
        asset.sub_meshes.emplace_back(mesh["mesh_index"].as<uint32_t>(), mesh["mesh_guid"].as<uint64_t>());
    }

    return asset;
}

gns::assets::AssetType gns::editor::assets::AssetImporter::GetAssetType(const std::string& extension)
{
	if (gns::assets::fileExtensionAssetTypeMap.contains(extension))
	    return gns::assets::fileExtensionAssetTypeMap[extension];

	return gns::assets::AssetType::None;
}

bool gns::editor::assets::AssetImporter::ImportMesh(std::string file_path, MeshImportOptions options, guid guid)
{
    LOG_INFO("Importing mesh: '" + file_path + "' ...");
    Assimp::Importer importer;
    try
    {
	    const aiScene* scene = importer.ReadFile(PathManager::FromAssetsRelative(file_path),
	        aiProcess_CalcTangentSpace |
	        aiProcess_Triangulate |
	        aiProcess_JoinIdenticalVertices |
	        aiProcess_SortByPType);

	    if (nullptr == scene) {
	        LOG_ERROR(importer.GetErrorString());
	        return false;
	    }

	    if (!scene->HasMeshes())
	        return false;

	    std::string assetname = fileUtils::GetFileNameFromPath(file_path);
        gns::assets::MeshAssetDescription meshAsset{guid, assetname , file_path, {}};

    	std::vector<gns::guid> materialGuids = {};
	    if (scene->HasMaterials() && options.import_materials)
	    {
	        for (size_t m = 0; m < scene->mNumMaterials; m++)
	        {
	            materialGuids.emplace_back(Guid::GetNewGuid());
	        }
	    }

	    for (size_t m = 0; m < scene->mNumMeshes; m++)
	    {
	        meshAsset.sub_meshes.emplace_back(m, Guid::GetNewGuid());
	    }

    	YAML::Emitter out_gnsMesh_file;
	    out_gnsMesh_file << YAML::BeginMap;
		out_gnsMesh_file << "asset_guid" << guid;
	    out_gnsMesh_file << "asset_name" << assetname;
	    out_gnsMesh_file << "file_path" << file_path;
	    out_gnsMesh_file << "sub_meshes" << YAML::BeginSeq;
	    for (gns::assets::SubMesh subMesh : meshAsset.sub_meshes)
	    {
	        out_gnsMesh_file << YAML::BeginMap;
	        out_gnsMesh_file << "mesh_index" << subMesh.mesh_index;
	        out_gnsMesh_file << "mesh_guid" << subMesh.mesh_guid;
	        out_gnsMesh_file << YAML::EndMap;
	    }
	    out_gnsMesh_file << YAML::EndSeq << YAML::EndMap;


	    {
			std::string gnsMeshFilePath = PathManager::FromAssetsRelative(file_path + ".gnsMesh");
	        std::ofstream outfile(gnsMeshFilePath);
	        outfile << out_gnsMesh_file.c_str() << std::endl;
	        outfile.close();
	    }
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("Failed to import Mesh: " + static_cast<std::string>(e.what()));
        return false;
    }
    
}

bool gns::editor::assets::AssetImporter::ImportTexture(const std::string& file_path, TextureImportOptions& out_options, const guid guid)
{

    std::string aPath = PathManager::FromAssetsRelative(file_path);
    if (!gns::fileUtils::FileExists(aPath))
        return false;
    int32_t channels;
    uint32_t width;
    uint32_t height;
    uint32_t depth = 1;
    bool hdr = gns::assets::AssetLoader::IsTextureHDR(aPath);

	if (!gns::assets::AssetLoader::ReadTextureData(aPath, width, height, channels))
        return false;

    //Create gnsTexture
    std::string assetname = fileUtils::GetFileNameFromPath(file_path);
    gns::assets::TextureAssetDescription textureAsset = {
	    {guid, assetname}, 
    	file_path, 
    	out_options.textureAssetType,
        hdr,
        width,
        height,
        depth
    };
    YAML::Emitter out;
    out << YAML::BeginMap;
    out << "asset_guid" << guid;
    out << "asset_name" << assetname;
    out << "file_path" << file_path;
    out << "texture_type" << static_cast<uint32_t>(out_options.textureAssetType);
    out << "width" << width;
    out << "height" << height;
    out << "depth" << depth;
    out << "hdr" << hdr;
    out <<  YAML::EndMap;


    {
        std::string gnsAssetFilePath = PathManager::FromAssetsRelative(file_path + ".gnsTex");
        std::ofstream outfile(gnsAssetFilePath);
        outfile << out.c_str() << "\n";
        outfile.close();
    }

}

bool gns::editor::assets::AssetImporter::ImportAssetInternal(
    const  gns::assets::AssetType assetType, const std::string& relative_path, const  gns::guid guid)
{
    bool import_result = false;

    switch (assetType) {
    case gns::assets::AssetType::None:
        break;
    case gns::assets::AssetType::Mesh:
        import_result = ImportMesh(relative_path, {}, guid);
        break;
    case gns::assets::AssetType::Texture:
    {
        TextureImportOptions texture_import_options = {};
        texture_import_options.textureAssetType = gns::assets::TextureAssetType::Texture2D;
        import_result = ImportTexture(relative_path, texture_import_options, guid);
    }
    break;
    case gns::assets::AssetType::Sound:
        break;
    case gns::assets::AssetType::Material:
        break;
    case gns::assets::AssetType::Shader:
        break;
    case gns::assets::AssetType::Compute:
        break;
    default:;
    }

    if (import_result)
    {
        std::string Extension = "";

        switch (assetType) {
        case gns::assets::AssetType::None:
            break;
        case gns::assets::AssetType::Mesh:
            Extension = ".gnsMesh";
            break;
        case gns::assets::AssetType::Texture:
            Extension = ".gnsTex";
            break;
        case gns::assets::AssetType::Sound:
            break;
        case gns::assets::AssetType::Material:
            break;
        case gns::assets::AssetType::Shader:
            break;
        case gns::assets::AssetType::Compute:
            break;
        }

        AssetLibrary::assetDatabase[guid] = {
            .assetGuid = guid,
            .assetName = fileUtils::GetFileNameFromPath(relative_path),
            .srcPath = relative_path + Extension,
            .assetType = assetType
        };



        YAML::Emitter meta_yaml;
        meta_yaml << YAML::BeginMap
            << "asset_guid" << AssetLibrary::assetDatabase[guid].assetGuid
            << "asset_name" << AssetLibrary::assetDatabase[guid].assetName
            << "src_path" << AssetLibrary::assetDatabase[guid].srcPath
            << "asset_type" << static_cast<uint32_t>(AssetLibrary::assetDatabase[guid].assetType) << YAML::EndMap;
        std::string meta_filePath = relative_path + ".meta";

        {
            std::ofstream outfile(PathManager::FromAssetsRelative(meta_filePath));
            outfile << meta_yaml.c_str() << std::endl;
            outfile.close();
        }

        YAML::Emitter database_yaml;
        database_yaml << YAML::BeginMap
            << "guid" << guid
            << "meta_path" << meta_filePath
            << YAML::EndMap;

        std::string DatabaseFilePath = PathManager::FromDatabaseRelative("." + std::to_string(guid));
        {
            std::ofstream outfile(DatabaseFilePath);
            outfile << database_yaml.c_str() << std::endl;
            outfile.close();
        }

        gns::assets::AssetRegistry::Add(guid, {
            gns::assets::AssetKind::Source, AssetLibrary::assetDatabase[guid].assetType, guid,  AssetLibrary::assetDatabase[guid].assetName,
            PathManager::AssetsPath + relative_path + Extension,
            0,0
            });

    }

    return import_result;
}
