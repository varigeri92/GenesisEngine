#include "gnspch.h"
#include "AssetLoader.h"
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include "../ECS/SystemsManager.h"
#include "../Rendering/RenderSystem.h"
#include "../ECS/Entity.h"
#include "../Rendering/Renderer.h"
#include "../ECS/Component.h"
#include "../Utils/FileSystemUtils.h"
#define STB_IMAGE_IMPLEMENTATION
#include "AssetMetadata.h"
#include "stb_image.h"
#include "../Utils/PathHelper.h"

std::unordered_map<gns::guid, gns::RuntimeAsset> gns::AssetRegistry::sRegistry = {};
gns::RuntimeAsset gns::AssetRegistry::sInvalidAssetEntry = {};

namespace gns::assetLibrary
{
struct AssetImportOptions
{
    bool ImportMaterials{ false };
    bool ImportSkeleton{ false };
};

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


std::unordered_map<aiTextureType, gns::rendering::TextureType> textureTypeMap = {
{aiTextureType_BASE_COLOR, rendering::TextureType::Albedo},
{aiTextureType_NORMALS, rendering::TextureType::Normal},
{aiTextureType_GLTF_METALLIC_ROUGHNESS, rendering::TextureType::MetallicRoughness},
{aiTextureType_AMBIENT_OCCLUSION, rendering::TextureType::AmbientOcclusion},
{aiTextureType_EMISSION_COLOR, rendering::TextureType::Emissive}
};

void LoadTextures(gns::RenderSystem* renderSystem, gns::rendering::Material* material, aiMaterial* mat, aiTextureType type, const std::string& asset_directory)
{
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        const std::string texture_path = asset_directory +"\\"+ str.C_Str();
        rendering::Texture* texture = renderSystem->CreateTexture(PathHelper::FromAssetsRelative(texture_path));
        if(texture == nullptr)
        {
            if (textureTypeMap[type] == rendering::TextureType::Normal)
                texture = renderSystem->GetDefaultNormalTexture();
            else
                texture = renderSystem->GetDefaultColorTexture();
        }
        material->textures[static_cast<uint32_t>(textureTypeMap[type])] = texture;
    }
}

void LoadEmbededTextures(gns::RenderSystem* renderSystem, gns::rendering::Material* material, aiMaterial* mat, aiTextureType type, const void* data)
{
    // TODO: :)
}


void LoadMeshAsset(const MeshAsset& mesh_asset, const std::function<void(const std::vector<guid>&, const std::vector<guid>&)>& onLoadSuccess_callback)
{
    std::string assetDir = gns::fileUtils::GetContainingDirectory(mesh_asset.src_path);
    if (assetDir == "")
        assetDir = PathHelper::AssetsPath;
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(PathHelper::FromAssetsRelative(mesh_asset.src_path),
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

    std::vector<guid> loaded_MeshGuids = {};
    loaded_MeshGuids.reserve(scene->mNumMeshes);
    std::vector<guid> loaded_materialGuids = {};
    loaded_materialGuids.reserve(scene->mNumMeshes);
	gns::RenderSystem* renderSystem = SystemsManager::GetSystem<gns::RenderSystem>();

	std::vector<rendering::Material*> materials = {};
    if (scene->HasMaterials())
    {
	    const std::string v_shader_path = R"(Shaders\colored_triangle_mesh.vert)";
	    const std::string f_shader_path = R"(Shaders\tex_image.frag)";
	    rendering::Shader* shader = renderSystem->CreateShader("default_shader", v_shader_path, f_shader_path);

        for (size_t m = 0; m < scene->mNumMaterials; m++)
        {
            aiMaterial* mat = scene->mMaterials[m];

            rendering::Material* material = renderSystem->CreateMaterial(shader, mat->GetName().C_Str());
            material->uniformData.metallic_roughness_AO = { 0,1,1,0 };
            renderSystem->ResetMaterialTextures(material);
            materials.push_back(material);

			LoadTextures(renderSystem, material, mat, aiTextureType_NORMALS, assetDir);
            LoadTextures(renderSystem, material, mat, aiTextureType_BASE_COLOR, assetDir);
            LoadTextures(renderSystem, material, mat, aiTextureType_EMISSIVE, assetDir);
            LoadTextures(renderSystem, material, mat, aiTextureType_GLTF_METALLIC_ROUGHNESS, assetDir);
            LoadTextures(renderSystem, material, mat, aiTextureType_AMBIENT_OCCLUSION, assetDir);
        }
    }

    for (size_t m = 0; m < scene->mNumMeshes; m++)
    {
        loaded_MeshGuids.push_back(mesh_asset.sub_meshes[m].mesh_guid);
        gns::rendering::Mesh* newMesh = gns::Object::CreateWithGuid<gns::rendering::Mesh>(
            mesh_asset.sub_meshes[m].mesh_guid, scene->mMeshes[m]->mName.C_Str());

        const aiMesh* mesh = scene->mMeshes[m];

    	if (scene->HasMaterials())
            loaded_materialGuids.emplace_back(materials[mesh->mMaterialIndex]->getGuid());

        for (size_t v = 0; v < scene->mMeshes[m]->mNumVertices; v++)
        {
            newMesh->positions.push_back({ mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z });
            newMesh->normals.push_back({ mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z });
            newMesh->colors.push_back({ mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z, 1.f });
            newMesh->uvs.push_back({ mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y * -1 });
        }
        if (mesh->HasTangentsAndBitangents())
        {
            for (size_t v = 0; v < scene->mMeshes[m]->mNumVertices; v++)
            {
                newMesh->tangents.push_back({ mesh->mTangents[v].x, mesh->mTangents[v].y, mesh->mTangents[v].z });
                newMesh->biTangents.push_back({ mesh->mBitangents[v].x, mesh->mBitangents[v].y, mesh->mBitangents[v].z });
            }
        }
        const uint32_t startindex = newMesh->indices.size();
        for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
            const aiFace& Face = mesh->mFaces[i];
            for (uint32_t i = 0; i < Face.mNumIndices; i++)
            {
                uint32_t vi = Face.mIndices[i];
                newMesh->indices.push_back(vi);
            }
        }
        const uint32_t count = static_cast<uint32_t>(newMesh->indices.size());
        renderSystem->UploadMesh(newMesh, startindex, count);
    }
    onLoadSuccess_callback(loaded_MeshGuids, loaded_materialGuids);
}
    /*
void ProcessAIScene(const aiScene* scene, std::string name)
{

    if (scene->HasMeshes())
    {
        Entity root_enntity = Entity::CreateEntity(name);

        std::vector<rendering::Material*> materials;
        gns::RenderSystem* renderSystem = SystemsManager::GetSystem<gns::RenderSystem>();
        const std::string v_shader_path = R"(Shaders\colored_triangle_mesh.vert)";
        const std::string f_shader_path = R"(GenesisEngine\Resources\Shaders\tex_image.frag)";

        rendering::Shader* shader = renderSystem->CreateShader(v_shader_path, f_shader_path);

    	if(scene->HasMaterials())
	        for (size_t m = 0; m < scene->mNumMaterials; m++)
	        {
                aiMaterial* mat = scene->mMaterials[m];
                LOG_INFO(mat->GetName().C_Str());
                rendering::Material* material = renderSystem->CreateMaterial(shader, mat->GetName().C_Str());
                material->uniformData.metallic_roughness_AO = { 0,1,1,0 };
                renderSystem->ResetMaterialTextures(material);
                materials.push_back(material);
	        }
        

    	for (size_t m = 0; m < scene->mNumMeshes; m++)
        {
			gns::Entity entity = gns::Entity::CreateEntity(scene->mMeshes[m]->mName.C_Str());
            root_enntity.AddChild(entity);
            gns::rendering::Mesh* newMesh = gns::Object::Create<gns::rendering::Mesh>(scene->mMeshes[m]->mName.C_Str());
            const aiMesh* mesh = scene->mMeshes[m];

            for (size_t v = 0; v < scene->mMeshes[m]->mNumVertices; v++)
            {
                newMesh->positions.push_back({ mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z });
                newMesh->normals.push_back({ mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z });
                newMesh->colors.push_back({ mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z, 1.f });
                newMesh->uvs.push_back({ mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y*-1});
            }
            if (mesh->HasTangentsAndBitangents())
            {
                for (size_t v = 0; v < scene->mMeshes[m]->mNumVertices; v++)
                {
                    newMesh->tangents.push_back({ mesh->mTangents[v].x, mesh->mTangents[v].y, mesh->mTangents[v].z });
                    newMesh->biTangents.push_back({ mesh->mBitangents[v].x, mesh->mBitangents[v].y, mesh->mBitangents[v].z });
                }
            }
            newMesh->indexBufferRange.startIndex = static_cast<uint32_t>(newMesh->indices.size());
            for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
                const aiFace& Face = mesh->mFaces[i];
                for (uint32_t i = 0; i < Face.mNumIndices; i++)
                {
                    uint32_t vi = Face.mIndices[i];;
                    newMesh->indices.push_back(vi);
                }
            }
            auto& meshComp = entity.AddComponet<gns::entity::MeshComponent>();
            meshComp.mesh_ref = newMesh->getGuid();
            meshComp.material_ref = materials[mesh->mMaterialIndex]->getGuid();
            newMesh->indexBufferRange.count = static_cast<uint32_t>(newMesh->indices.size());
        }
    }
}
*/

	void LoadMesh(const std::string& filePath)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filePath,
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType);

    if (nullptr == scene) {
        LOG_ERROR(importer.GetErrorString());
        return;
    }
	//ProcessAIScene(scene, gns::fileUtils::GetFileNameFromPath(filePath));
}

void LoadMaterial(const std::string& filePath)
{

}

void LoadTexture(const std::string& filePath, rendering::Texture& texture)
{
    int32_t channels;
    int32_t chan = 4;

    texture.data = stbi_load(filePath.c_str(), (int32_t*) & texture.width, (int32_t*)&texture.height, &channels, chan);
}

void LoadAsset(const std::string& filePath)
{
    AssetType type = fileExtensionAssetTypeMap[fileUtils::GetFileExtension(filePath)];

    switch (type)
    {
    case AssetType::Mesh:
		LoadMesh(filePath);
         break;
    case AssetType::Texture:
        LOG_WARNING("Not Implemented!");
        break;
    case AssetType::Sound:
        LOG_WARNING("Not Implemented!");
	    break;
    case AssetType::Material:
        LOG_WARNING("Not Implemented!");
	    break;
    default:
        LOG_ERROR("Asset type of File: '" + filePath + "' Is Not Supported!");
        break;
    }
}
}
