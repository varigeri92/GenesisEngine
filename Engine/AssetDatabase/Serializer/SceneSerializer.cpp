#include "gnspch.h"
#include "SceneSerializer.h"
#include "Genesis.h"
#include "GenesisRendering.h"
#include "GenesisSystems.h"
#define YAML_CPP_STATIC_DEFINE
#include "../../Scene/Scene.h"
#include "../../Scene/SceneManager.h"
#include "../../Utils/FileSystemUtils.h"
#include "yaml-cpp/yaml.h"
#include "SerializationTable.h"

#define VERSION "0.0.0"
using namespace gns::serialization;

std::unordered_map<size_t, gns::serialization::YamlFieldSerializationEntry>
	gns::serialization::YamlFieldSerializationEntry::yaml_table = {};
std::unordered_map<size_t, gns::serialization::YamlComponentSerializationEntry>
gns::serialization::YamlComponentSerializationEntry::yaml_table = {};

/*
std::unordered_map<size_t, std::function<void(const std::string& name, char* componentData, size_t offset, YAML::Emitter& out)>>
FieldSerializerTable
{
	{typeid(std::string).hash_code(),[](const std::string& name, char* componentData, size_t offset, YAML::Emitter& out)
		{
			out << YAML::Key << name
			<< YAML::Value << *reinterpret_cast<std::string*>(componentData + offset);
		}
	},
	{typeid(glm::vec3).hash_code(),[](const std::string& name, char* componentData, size_t offset , YAML::Emitter& out)
		{
			glm::vec3 vec3 = *reinterpret_cast<glm::vec3*>(componentData + offset);
			out << YAML::Key << name
			<< YAML::Value << YAML::Flow << YAML::BeginSeq << vec3.x << vec3.y << vec3.z << YAML::EndSeq;
		}
	},
	{typeid(gns::guid).hash_code(),[](const std::string& name, char* componentData, size_t offset, YAML::Emitter& out)
		{
			out << YAML::Key << name
			<< YAML::Value << *reinterpret_cast<size_t*>(componentData + offset);
		}
	},
	{typeid(bool).hash_code(),[](const std::string& name, char* componentData, size_t offset, YAML::Emitter& out)
		{
			bool b = *reinterpret_cast<bool*>(componentData + offset);
			out << YAML::Key << name
			<< YAML::Value << b;
		}
	},
	{typeid(float).hash_code(),[](const std::string& name, char* componentData, size_t offset, YAML::Emitter& out)
		{
			float f = *reinterpret_cast<float*>(componentData + offset);
			out << YAML::Key << name
				<< YAML::Value << f;
		}
	},
	{typeid(gns::color4).hash_code(),[](const std::string& name, char* componentData, size_t offset, YAML::Emitter& out)
		{
			glm::vec4 vec4 = *reinterpret_cast<glm::vec4*>(componentData + offset);
			out << YAML::Key << name
				<< YAML::Value << YAML::Flow << YAML::BeginSeq << vec4.x << vec4.y << vec4.z << vec4.w << YAML::EndSeq;
		}
	},
	{typeid(gns::color3).hash_code(),[](const std::string& name, char* componentData, size_t offset, YAML::Emitter& out)
		{
			glm::vec3 vec3 = *reinterpret_cast<glm::vec3*>(componentData + offset);
			out << YAML::Key << name
				<< YAML::Value << YAML::Flow << YAML::BeginSeq << vec3.x << vec3.y << vec3.z << YAML::EndSeq;
		}
	},
};
std::unordered_map<size_t, std::function<void(char* componentData, size_t offset, YAML::Node& value)>>
FieldDeserializer_Table
{
{
	typeid(std::string).hash_code(),
		[](char* componentData, size_t offset, YAML::Node& value)
		{
			std::string& _value = *reinterpret_cast<std::string*>(componentData + offset);
			_value = value.as<std::string>();
		}
},
{ typeid(glm::vec3).hash_code(),
[](char* componentData, size_t offset, YAML::Node& value)
	{
		glm::vec3& _value = *reinterpret_cast<glm::vec3*>(componentData + offset);
		for (int i = 0; i < 3; i++)
		{
			_value[i] = value[i].as<float>();
		}
	}
}
};

std::unordered_map<size_t, std::function<void* (gns::entityHandle entity_handle)>>
ComponentDeserializer_Table
{
	{static_cast<size_t>(entt::type_hash<gns::entity::Transform>::value()),
	[](gns::entityHandle entity_handle)
		{
			gns::Entity entity = {entity_handle};
			auto& component = entity.GetComponent<gns::entity::Transform>();
			return static_cast<void*>(&component);
		}
	},

	{static_cast<size_t>(entt::type_hash<gns::entity::EntityComponent>::value()),
	[](gns::entityHandle entity_handle)
		{
			gns::Entity entity = {entity_handle};
			auto& component = entity.GetComponent<gns::entity::EntityComponent>();
			return static_cast<void*>(&component);
		}
	},

	{static_cast<size_t>(entt::type_hash<gns::rendering::PointLightComponent>::value()),
	[](gns::entityHandle entity_handle)
		{
			gns::Entity entity = {entity_handle};
			auto& component = entity.AddComponet<gns::rendering::PointLightComponent>();
			return static_cast<void*>(&component);
		}
	},
	{static_cast<size_t>(entt::type_hash<gns::rendering::ColorComponent>::value()),
	[](gns::entityHandle entity_handle)
		{
			gns::Entity entity = {entity_handle};
			auto& component = entity.AddComponet<gns::rendering::ColorComponent>();
			return static_cast<void*>(&component);
		}
	},
	{static_cast<size_t>(entt::type_hash<gns::entity::MeshComponent>::value()),
	[](gns::entityHandle entity_handle)
		{
			gns::Entity entity = {entity_handle};
			auto& component = entity.AddComponet<gns::entity::MeshComponent>();
			return static_cast<void*>(&component);
		}
	}
};
*/

void gns::serialization::SceneSerializer::RegisterTable()
{
	std::function<void(const std::string& name, size_t typeId, char* componentData, YAML::Emitter& out)> serializeFn =
		[](const std::string& name, size_t typeId, char* componentData, YAML::Emitter& out)
		{
			out << YAML::BeginMap << YAML::Key << "component_data" << YAML::Value << YAML::BeginMap;
			out << YAML::Key << "cmp_name" << YAML::Value << name;
			out << YAML::Key << "cmp_type" << YAML::Value << typeId;
			out << YAML::Key << "cmp_fields" << YAML::Value;
		};

	YamlComponentSerializationEntry::RegisterSerializableComponent<gns::entity::Transform>(
		serializeFn,
		[](gns::entityHandle entity_handle, YAML::Node& value)
		{
			gns::Entity entity = { entity_handle };
			auto& component = entity.GetComponent<gns::entity::Transform>();
			return static_cast<void*>(&component);
		}
	);

	YamlComponentSerializationEntry::RegisterSerializableComponent<gns::entity::EntityComponent>(
		serializeFn,
		[](gns::entityHandle entity_handle, YAML::Node& value)
		{
			gns::Entity entity = { entity_handle };
			auto& component = entity.GetComponent<gns::entity::EntityComponent>();
			return static_cast<void*>(&component);
		});

	YamlComponentSerializationEntry::RegisterSerializableComponent<gns::rendering::PointLightComponent>(
		serializeFn,
		[](gns::entityHandle entity_handle, YAML::Node& value)
		{
			gns::Entity entity = { entity_handle };
			auto& component = entity.AddComponet<gns::rendering::PointLightComponent>();
			return static_cast<void*>(&component);
		});

	YamlComponentSerializationEntry::RegisterSerializableComponent<gns::rendering::ColorComponent>(
		serializeFn,
		[](gns::entityHandle entity_handle, YAML::Node& value)
		{
			gns::Entity entity = { entity_handle };
			auto& component = entity.AddComponet<gns::rendering::ColorComponent>();
			return static_cast<void*>(&component);
		});

	YamlComponentSerializationEntry::RegisterSerializableComponent<gns::entity::MeshComponent>(
		serializeFn,
		[](gns::entityHandle entity_handle, YAML::Node& value)
		{
			gns::Entity entity = { entity_handle };
			auto& component = entity.AddComponet<gns::entity::MeshComponent>();
			component.meshAsset = 0;
			component.material_ref = 0;
			return static_cast<void*>(&component);
		});
	YamlComponentSerializationEntry::RegisterSerializableComponent<gns::rendering::LightComponent>(
		serializeFn,
		[](gns::entityHandle entity_handle, YAML::Node& value)
		{
			gns::Entity entity = { entity_handle };
			auto& component = entity.AddComponet<gns::rendering::LightComponent>();
			return static_cast<void*>(&component);
		});


	//-----------------------------------------
	//------------ FIELDS: --------------------
	//-----------------------------------------

	YamlFieldSerializationEntry::RegisterSerializeableFiled<std::string>(
		[](const std::string& name, char* componentData, size_t offset, YAML::Emitter& out)
		{
			out << YAML::Key << name
				<< YAML::Value << *reinterpret_cast<std::string*>(componentData + offset);
		},
		[](char* componentData, size_t offset, YAML::Node& value)
		{
			std::string& _value = *reinterpret_cast<std::string*>(componentData + offset);
			_value = value.as<std::string>();
		}
	);

	YamlFieldSerializationEntry::RegisterSerializeableFiled<glm::vec3>(
		[](const std::string& name, char* componentData, size_t offset, YAML::Emitter& out)
		{
			glm::vec3 vec3 = *reinterpret_cast<glm::vec3*>(componentData + offset);
			out << YAML::Key << name
				<< YAML::Value << YAML::Flow << YAML::BeginSeq << vec3.x << vec3.y << vec3.z << YAML::EndSeq;
		},
		[](char* componentData, size_t offset, YAML::Node& value)
		{
			glm::vec3& _value = *reinterpret_cast<glm::vec3*>(componentData + offset);
			for (int i = 0; i < 3; i++)
			{
				_value[i] = value[i].as<float>();
			}
		}
	);

	YamlFieldSerializationEntry::RegisterSerializeableFiled<gns::guid>(
		[](const std::string& name, char* componentData, size_t offset, YAML::Emitter& out)
		{
			out << YAML::Key << name
				<< YAML::Value << *reinterpret_cast<size_t*>(componentData + offset);
		},[](char* componentData, size_t offset, YAML::Node& value)
		{
				gns::guid& _value = *reinterpret_cast<gns::guid*>(componentData + offset);
				_value = value.as<gns::guid>();
		}
	);

	YamlFieldSerializationEntry::RegisterSerializeableFiled<bool>(
		[](const std::string& name, char* componentData, size_t offset, YAML::Emitter& out)
		{
			bool b = *reinterpret_cast<bool*>(componentData + offset);
			out << YAML::Key << name
				<< YAML::Value << b;
		},
		[](char* componentData, size_t offset, YAML::Node& value)
		{
				bool& _value = *reinterpret_cast<bool*>(componentData + offset);
				_value = value.as<bool>();
		}
	);

	YamlFieldSerializationEntry::RegisterSerializeableFiled<float>(
		[](const std::string& name, char* componentData, size_t offset, YAML::Emitter& out)
		{
			float f = *reinterpret_cast<float*>(componentData + offset);
				out << YAML::Key << name
				<< YAML::Value << f;
		},
		[](char* componentData, size_t offset, YAML::Node& value)
		{
			float& _value = *reinterpret_cast<float*>(componentData + offset);
			_value = value.as<float>();
		}
	);

	YamlFieldSerializationEntry::RegisterSerializeableFiled<gns::color4>(
		[](const std::string& name, char* componentData, size_t offset, YAML::Emitter& out)
		{
			glm::vec4 vec4 = *reinterpret_cast<glm::vec4*>(componentData + offset);
			out << YAML::Key << name
				<< YAML::Value << YAML::Flow << YAML::BeginSeq << vec4.x << vec4.y << vec4.z << vec4.w << YAML::EndSeq;
		},
		[](char* componentData, size_t offset, YAML::Node& value)
		{
			glm::vec4& _value = *reinterpret_cast<glm::vec4*>(componentData + offset);
			for (int i = 0; i < 4; i++)
			{
				_value[i] = value[i].as<float>();
			}
		}
	);
	YamlFieldSerializationEntry::RegisterSerializeableFiled<gns::color3>(
		[](const std::string& name, char* componentData, size_t offset, YAML::Emitter& out)
		{
			glm::vec3 vec3 = *reinterpret_cast<glm::vec3*>(componentData + offset);
			out << YAML::Key << name
				<< YAML::Value << YAML::Flow << YAML::BeginSeq << vec3.x << vec3.y << vec3.z << YAML::EndSeq;
		},
		[](char* componentData, size_t offset, YAML::Node& value)
		{
			glm::vec3& _value = *reinterpret_cast<glm::vec3*>(componentData + offset);
			for (int i = 0; i < 3; i++)
			{
				_value[i] = value[i].as<float>();
			}
		}
	);
}


gns::MeshAsset GetMeshAsset(const std::string& filePath)
{
	std::string path = filePath;
	if(!gns::fileUtils::HasFileExtension(filePath, "gnsMesh"))
	{
		path = filePath + ".gnsMesh";
	}
	YAML::Node meshAssetFile = YAML::LoadFile(path);
	gns::MeshAsset asset = {
		meshAssetFile["asset_guid"].as<uint64_t>(),
		meshAssetFile["asset_name"].as<std::string>(),
		meshAssetFile["file_path"].as<std::string>(), {} };

	for (const auto& mesh : meshAssetFile["sub_meshes"])
	{
		asset.sub_meshes.emplace_back(mesh["mesh_index"].as<uint32_t>(), mesh["mesh_guid"].as<uint64_t>());
	}

	return asset;
}

void SceneSerializer::ProcessSceneReferences(gns::scene::Scene* scene)
{
	for (const auto & entity_handle : scene->m_entities)
	{
		Entity entity{ entity_handle };
		gns::entity::MeshComponent* meshComp;
		if(entity.TryGetComponent<gns::entity::MeshComponent>(meshComp))
		{
			RuntimeAsset runtimeAsset = AssetRegistry::Get(meshComp->meshAsset);
			MeshAsset meshAsset = GetMeshAsset(runtimeAsset.path);
			assetLibrary::LoadMeshAsset(meshAsset, 
				[&](const std::vector<guid>& loadedMeshes, const std::vector<guid>& loadedMaterials)
			{
				for (size_t i = 0; i < loadedMeshes.size(); i++)
				{
					meshComp->meshes.push_back(Object::Get<rendering::Mesh>(loadedMeshes[i]));
					meshComp->materials.push_back(Object::Get<rendering::Material>(loadedMaterials[i]));
				}
			});
		}
		
	}
}

void TrySetFieldValue(size_t field_type, char* componentData, size_t offset, YAML::Node& value)
{
	if(!YamlFieldSerializationEntry::Deserialize(field_type, componentData, offset, value))
	{
		LOG_ERROR("Setting field value failed!");
	}
}

void* TryAddComponent(size_t component_type_id, gns::entityHandle entity_handle, YAML::Node& value)
{
	void* componentPtr = YamlComponentSerializationEntry::Deserialize(component_type_id, entity_handle, value);
	if (componentPtr != nullptr)
		return componentPtr;

	LOG_ERROR("Cannot deserialize / add component!");
	return nullptr;
}

void TrySerializeField(size_t typeHash, const std::string& name, char* componentData, size_t offset, YAML::Emitter& out)
{
	if(!YamlFieldSerializationEntry::Serialize(typeHash, name, componentData, offset, out))
	{
		out << YAML::BeginMap;
		out << YAML::Key << name;
		out << YAML::Value << "Cant serialize Field value";
		out << YAML::EndMap;
	}
}

bool gns::serialization::SceneSerializer::SaveScene(const std::string& outFilePath)
{
	const std::string sceneYamlData = SerializeScene(&gns::scene::SceneManager::GetActiveScene());
	gns::fileUtils::CreateFile(outFilePath, sceneYamlData);
	return true;
}

std::string gns::serialization::SceneSerializer::SerializeScene(gns::scene::Scene* scene)
{
	YAML::Emitter out;
	out << YAML::BeginMap;
	out << YAML::Key << "serializer_version";
	out << YAML::Value << VERSION;

	out << YAML::Key << "scene_name";
	out << YAML::Value << scene->m_name;

	out << YAML::Key << "scene_data";
	out << YAML::Value << YAML::BeginSeq;
	for (entityHandle entity_handle : scene->m_entities)
	{
		Entity entity = Entity(entity_handle);
		std::vector<ComponentData> entityComponents = entity.GetAllComponent(); // getSerializeableComponents

		out << YAML::BeginMap;
		out << YAML::Key << "entity_name" << YAML::Value << entity.Name();
		out << YAML::Key << "entity_guid" << YAML::Value << entity.GetGuid();
		out << YAML::Key << "children";
		out << YAML::Value << YAML::BeginSeq;
		for (entityHandle child : entity.Children())
		{
			out << Entity(child).GetGuid();
		}
		out << YAML::EndSeq;


		out << YAML::Key << "parent" << YAML::Value << entity.Parent().GetGuid();

		out << YAML::Key << "components" << YAML::Value << YAML::BeginSeq;

		for (ComponentData& entityComponent : entityComponents)
		{
			size_t typehash = entityComponent.typehash;
			char* data_ptr = (char*)entityComponent.data;
			ComponentMeta& cmpMeta = ISerializeableComponent::sComponentData[typehash];

			YamlComponentSerializationEntry::Serialize(typehash, cmpMeta.name, data_ptr, out);
			out << YAML::BeginSeq;
			for (auto& field : cmpMeta.fields)
			{
				out << YAML::BeginMap;
				out << YAML::Key << "name" << YAML::Value << field.name;
				out << YAML::Key << "type" << YAML::Value << field.type;
				out << YAML::Key << "offset" << YAML::Value << field.offset;
				TrySerializeField(field.type, "field_value", data_ptr, field.offset, out);
				out << YAML::EndMap;
			}
			out << YAML::EndSeq << YAML::EndMap << YAML::EndMap;
		}
		out << YAML::EndSeq << YAML::EndMap;
	}
	out << YAML::EndSeq;
	out << YAML::EndMap;
	return out.c_str();
}


gns::scene::Scene* gns::serialization::SceneSerializer::DeserializeScene(const std::string& sceneFilePath)
{
	YAML::Node root = YAML::LoadFile(sceneFilePath);
	gns::scene::Scene* newScene = scene::SceneManager::CreateScene(root["scene_name"].as<std::string>());
	const std::string versionString = VERSION;
	if(root["serializer_version"].as<std::string>() != versionString)
	{
		LOG_WARNING("Serializer Version Missmatch!");
	}
	for (auto yaml_entity : root["scene_data"])
	{
		std::string name = yaml_entity["entity_name"].as<std::string>();
		gns::guid guid = yaml_entity["entity_guid"].as<size_t>();
		Entity entity = Entity::CreateEntity_Internal(name, guid, newScene);
		for (const auto & yaml_component_data : yaml_entity["components"])
		{
			auto yaml_component = yaml_component_data["component_data"];
			void* component_data_ptr = TryAddComponent(yaml_component["cmp_type"].as<size_t>(), entity, yaml_component);

			if(component_data_ptr == nullptr)
				continue;

			for (const auto & yaml_field : yaml_component["cmp_fields"])
			{
				size_t offset = yaml_field["offset"].as<size_t>();
				size_t type = yaml_field["type"].as<size_t>();
				YAML::Node field_value = yaml_field["field_value"];
				TrySetFieldValue(type, static_cast<char*>(component_data_ptr), offset, field_value);
			}
		}
	}
	scene::SceneManager::SetActiveScene(newScene);
	ProcessSceneReferences(newScene);
	return newScene;
}