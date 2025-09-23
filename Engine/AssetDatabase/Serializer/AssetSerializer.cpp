#include "gnspch.h"
#include "AssetSerializer.h"
#include "../../Rendering/Objects/Material.h"
#include "YamlOperatorHelper.h"
#include "../../../Editor/PathManager.h"
#include "../../Rendering/Shader.h"
#include "../../Utils/FileSystemUtils.h"
#include "../../Utils/PathHelper.h"

void AssetSerializer::SerializeMaterial(gns::rendering::Material* material)
{
	YAML::Emitter out;
	out << YAML::BeginMap;
	out << "asset_guid" << material->getGuid();
	out << "vertex_shader" << material->shader->vertexShaderPath;
	out << "fragment_shader" << material->shader->fragmentShaderPath;
	out << "albedo_color" << material->uniformData.albedoColor;
	out << "metallic_roughness_AO" << material->uniformData.metallic_roughness_AO;
	out << "textures" << YAML::BeginSeq;
	for (size_t i = 0; i < material->textures.size(); i++)
	{
		out << material->textures[i]->getGuid();
	}
	out << YAML::EndMap;

	gns::fileUtils::CreateFile(gns::PathHelper::FromAssetsRelative(R"(material.gnsMat)"), out.c_str());
}
