﻿#include "AssetImporter.h"
#include <iostream>
#include "GenesisRendering.h"
#include "AssetLibrary.h"
#include "Genesis.h"
#include "../PathManager.h"
#include "../../Engine/Utils/FileSystemUtils.h"
#define YAML_CPP_STATIC_DEFINE
#include <fstream>
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include "yaml-cpp/yaml.h"

namespace gns::assetLibrary
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
            guid = YAML::LoadFile(PathManager::FromAssetsRelative(relative_path + ".meta"))["asset_guid"].as<size_t>();
        else
            return true;
    }

    AssetLibrary::assetDatabase[guid] = {
        .assetGuid = guid,
        .assetName = fileUtils::GetFileNameFromPath(relative_path),
        .srcPath = relative_path,
        .assetType = assets::AssetImporter::GetAssetType(fileUtils::GetFileExtension(relative_path))
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
    switch (AssetLibrary::assetDatabase[guid].assetType) {
    case assetLibrary::AssetType::None:
	    break;
    case assetLibrary::AssetType::Mesh:
        ImportMesh(relative_path, {}, guid);
	    break;
    case assetLibrary::AssetType::Texture:
	    break;
    case assetLibrary::AssetType::Sound:
	    break;
    case assetLibrary::AssetType::Material:
	    break;
    case assetLibrary::AssetType::Shader:
	    break;
    case assetLibrary::AssetType::Compute:
	    break;
    default: ;
    }

    return true;
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

void gns::editor::assets::AssetImporter::OpenImportWindow(assetLibrary::AssetType type)
{

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
            static_cast<assetLibrary::AssetType>(metaFile["asset_type"].as<uint32_t>())
        };
        return &AssetLibrary::assetDatabase[asset_guid];
    }
}

gns::MeshAsset gns::editor::assets::AssetImporter::GetMeshAsset(const AssetMetadata& asset_metadata)
{
    YAML::Node meshAssetFile = YAML::LoadFile(PathManager::FromAssetsRelative(asset_metadata.srcPath) + ".gnsMesh");
    MeshAsset asset = {meshAssetFile["asset_guid"].as<uint64_t>(),
    meshAssetFile["asset_name"].as<std::string>(),
    meshAssetFile["file_path"].as<std::string>(), {}};

    for (const auto & mesh : meshAssetFile["sub_meshes"])
    {
        asset.sub_meshes.emplace_back(mesh["mesh_index"].as<uint32_t>(), mesh["mesh_guid"].as<uint64_t>());
    }

    return asset;
}

gns::assetLibrary::AssetType gns::editor::assets::AssetImporter::GetAssetType(const std::string& extension)
{
	if (assetLibrary::fileExtensionAssetTypeMap.contains(extension))
	    return assetLibrary::fileExtensionAssetTypeMap[extension];

	return assetLibrary::AssetType::None;
}

void gns::editor::assets::AssetImporter::ImportMesh(std::string file_path, MeshImportOptions options, guid guid)
{
    LOG_INFO("Importing mesh: '" + file_path + "' ...");
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(PathManager::FromAssetsRelative(file_path),
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType);

    if (nullptr == scene) {
        LOG_ERROR(importer.GetErrorString());
        return;
    }

    if (!scene->HasMeshes())
        return;

    if (scene->HasMaterials())
    {
        std::vector<rendering::Material*> materials;
        const std::string v_shader_path = R"(Shaders\colored_triangle_mesh.vert)";
        const std::string f_shader_path = R"(Shaders\tex_image.frag)";

        for (size_t t = 0; t < scene->mNumTextures; t++)
        {
            LOG_INFO(scene->mTextures[t]->mFilename.C_Str());
        }

        for (size_t m = 0; m < scene->mNumMaterials; m++)
        {
            aiMaterial* mat = scene->mMaterials[m];
            /*
             
            rendering::Material* material = renderSystem->CreateMaterial(shader, mat->GetName().C_Str());
            material->uniformData.metallic_roughness_AO = { 0,1,1,0 };
            renderSystem->ResetMaterialTextures(material);
            materials.push_back(material);

            LoadTextures(mat, aiTextureType_NORMALS);
            LoadTextures(mat, aiTextureType_BASE_COLOR);
            LoadTextures(mat, aiTextureType_EMISSIVE);
            LoadTextures(mat, aiTextureType_GLTF_METALLIC_ROUGHNESS);
            LoadTextures(mat, aiTextureType_METALNESS);
            LoadTextures(mat, aiTextureType_DIFFUSE_ROUGHNESS);
            LoadTextures(mat, aiTextureType_SPECULAR);
            */
        }
    }


    std::string assetname = fileUtils::GetFileNameFromPath(file_path);
    MeshAsset meshAsset{guid, assetname , file_path, {}};
    std::vector<gns::guid> materialGuids = {};
    if (scene->HasMaterials() && options.import_materials)
    {
        for (size_t m = 0; m < scene->mNumMaterials; m++)
        {
            aiMaterial* mat = scene->mMaterials[m];
            materialGuids.emplace_back(Guid::GetNewGuid());
        }
    }

    for (size_t m = 0; m < scene->mNumMeshes; m++)
    {
        const aiMesh* mesh = scene->mMeshes[m];
        meshAsset.sub_meshes.emplace_back(m, Guid::GetNewGuid());
    }
    YAML::Emitter out;
    out << YAML::BeginMap;
	out << "asset_guid" << guid;
    out << "asset_name" << assetname;
    out << "file_path" << file_path;
    out << "sub_meshes" << YAML::BeginSeq;
    for (AssetSubmesh subMesh : meshAsset.sub_meshes)
    {
        out << YAML::BeginMap;
        out << "mesh_index" << subMesh.mesh_index;
        out << "mesh_guid" << subMesh.mesh_guid;
        out << YAML::EndMap;
    }
    out << YAML::EndSeq << YAML::EndMap;


    std::string DatabaseFilePath = PathManager::FromAssetsRelative(file_path + ".gnsMesh");
    {
        std::ofstream outfile(DatabaseFilePath);
        outfile << out.c_str() << std::endl;
        outfile.close();
    }
    
}
