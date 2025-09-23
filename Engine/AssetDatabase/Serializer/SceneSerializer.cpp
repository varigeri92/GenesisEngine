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


#define VERSION "0.0.0";


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
			bool f = *reinterpret_cast<float*>(componentData + offset);
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
			out << YAML::BeginMap
			<< YAML::Key << name
			<< YAML::Value << YAML::Flow << YAML::BeginSeq << vec3.x << vec3.y << vec3.z << YAML::EndSeq
			<< YAML::EndMap;
		}
	},
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
	}
};

std::unordered_map<size_t, std::function<void(char* componentData, size_t offset, YAML::Node& value)>>
FieldDeserializer_Table
{
	/*
	 */
	{typeid(std::string).hash_code(),
	[](char* componentData, size_t offset, YAML::Node& value)
		{
			std::string& _value = *reinterpret_cast<std::string*>(componentData + offset);
			_value = value.as<std::string>();
		}
	},
	{typeid(glm::vec3).hash_code(),
	[](char* componentData, size_t offset, YAML::Node& value)
		{
			glm::vec3& _value = *reinterpret_cast<glm::vec3*>(componentData + offset);
			for(int i = 0; i<3; i++)
			{
				_value[i] = value[i].as<float>();
			}
		}
	}
};


void TrySetFieldValue(size_t field_type, char* componentData, size_t offset, YAML::Node& value)
{
	if (FieldDeserializer_Table.contains(field_type))
	{
		FieldDeserializer_Table[field_type](componentData, offset, value);
	}
}

void* TryAddComponent(size_t component_type_id, gns::entityHandle entity_handle)
{
	if(ComponentDeserializer_Table.contains(component_type_id))
	{
		return ComponentDeserializer_Table[component_type_id](entity_handle);
	}
	return nullptr;
	
}

void TrySerializeField(size_t typeHash, const std::string& name, char* componentData, size_t offset, YAML::Emitter& out)
{
	if(FieldSerializerTable.contains(typeHash))
		FieldSerializerTable[typeHash](name,componentData,offset,out);
	else
	{
		out << YAML::BeginMap;
		out << YAML::Key << name;
		out << YAML::Value << "Cant serialize Field value";
		out << YAML::EndMap;
	}
}

bool gns::editor::serialization::SceneSerializer::SaveScene(const std::string& outFilePath)
{
	const std::string sceneYamlData = SerializeScene(&scene::SceneManager::GetActiveScene());
	gns::fileUtils::CreateFile(outFilePath, sceneYamlData);
	return true;
}

std::string gns::editor::serialization::SceneSerializer::SerializeScene(scene::Scene* scene)
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

			out << YAML::BeginMap << YAML::Key << "component_data" << YAML::Value << YAML::BeginMap;
			out << YAML::Key << "cmp_name" << YAML::Value << cmpMeta.name;
			out << YAML::Key << "cmp_type" << YAML::Value << typehash;
			out << YAML::Key << "cmp_fields" << YAML::Value << YAML::BeginSeq;
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

gns::scene::Scene* gns::editor::serialization::SceneSerializer::DeserializeScene(const std::string& sceneFilePath)
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
			void* component_data_ptr = TryAddComponent(yaml_component["cmp_type"].as<size_t>(), entity);
			if(component_data_ptr == nullptr)
				continue;

			for (const auto & yaml_field : yaml_component["cmp_fields"])
			{
				//LOG_INFO(yaml_field["name"].as<std::string>());
				//LOG_INFO(yaml_field["type"].as<std::string>());
				//LOG_INFO(yaml_field["offset"].as<std::string>());
				//LOG_INFO(yaml_field["offset"].as<std::string>());
				size_t offset = yaml_field["offset"].as<size_t>();
				size_t type = yaml_field["type"].as<size_t>();

				YAML::Node field_value = yaml_field["field_value"];

				TrySetFieldValue(type, static_cast<char*>(component_data_ptr), offset, field_value);
			}
		}
	}
	return newScene;
}

