#pragma once
#include <glm/ext/scalar_uint_sized.hpp>

#include "../Object/Guid.h"
#include "../Utils/Logger.h"
#include "AssetTypes.h"

namespace gns::assets
{
	struct AssetInfo
	{
		AssetKind assetKind{ AssetKind::Invalid };
		AssetType AssetType{ AssetType::None };
		guid assetGuid;
		std::string name;
		std::string filePath{};
		size_t offset{0};
		size_t size{0};
	};
	class AssetRegistry
	{
	private:
		GNS_API static AssetInfo sInvalidAssetEntry;
		GNS_API static std::unordered_map<guid, AssetInfo> sRegistry;
	public:
		static const AssetInfo& Get(guid guid)
		{
			if(sRegistry.contains(guid))
				return sRegistry[guid];

			return sInvalidAssetEntry;
		}

		static void Add(guid guid, AssetInfo asset)
		{
			if(sRegistry.contains(guid))
			{
				LOG_INFO("Guid Collision... overvrite entry.");
			}
			sRegistry[guid] = std::move(asset);
		}

		static void ListAssets()
		{
			for (auto& it : sRegistry) {
				LOG_INFO(it.second.filePath);
			}
		}
	};
	struct AssetDescriptionHeader
	{
		guid assetGuid;
		std::string assetName;
	};

	struct MaterialAssetDescription
	{
		AssetDescriptionHeader assetHeader;
		std::string vert_shader_path;
		std::string frag_shader_path;
	};

	struct SubMesh
	{
		uint32_t mesh_index;
		guid mesh_guid;
	};

	struct MeshAssetDescription
	{
		AssetDescriptionHeader assetHeader;
		std::string src_path;
		bool isStatic;
		std::vector<SubMesh> sub_meshes;
	};

	struct TextureAssetDescription
	{
		AssetDescriptionHeader assetHeader;
		std::string src_path;
		TextureAssetType textureType;
		bool hdr;
		uint32_t width;
		uint32_t height;
		uint32_t depth{1};
	};
}