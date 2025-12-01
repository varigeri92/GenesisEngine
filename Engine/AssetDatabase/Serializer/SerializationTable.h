#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include <yaml-cpp/yaml.h>


namespace gns::serialization
{
	struct YamlFieldSerializationEntry
	{
		static std::unordered_map<size_t, YamlFieldSerializationEntry> yaml_table;
	private:
		std::function<void (const std::string& name, char* componentData, size_t offset, YAML::Emitter& out)> serialize{ nullptr };
		std::function<void (char* componentData, size_t offset, YAML::Node& value)> deserialize{ nullptr };

		YamlFieldSerializationEntry(
			std::function<void (const std::string& name, char* componentData, size_t offset, YAML::Emitter& out)> serializeFn,
			std::function<void (char* componentData, size_t offset, YAML::Node& value)> deserializeFn)
		{
			serialize = serializeFn;
			deserialize = deserializeFn;
		}
	public:
		YamlFieldSerializationEntry() = default;

		template<typename T>
		static void RegisterSerializeableFiled(
			std::function<void (const std::string& name, char* componentData, size_t offset, YAML::Emitter& out)> serializeFn,
			std::function<void (char* componentData, size_t offset, YAML::Node& value)> deserializeFn)
		{
			size_t typeId = typeid(T).hash_code();
			if(yaml_table.contains(typeId))
			{
				return;
			}
			YamlFieldSerializationEntry e = { serializeFn, deserializeFn };
			yaml_table.emplace(typeId, std::move(e));
		}

		template<typename T>
		static bool Serialize(const std::string& name, char* componentData, size_t offset, YAML::Emitter& out)
		{
			size_t typeId = typeid(T).hash_code();
			if (yaml_table.contains(typeId))
			{
				if(yaml_table[typeId].serialize == nullptr)
					return false;

				yaml_table[typeId].serialize(name, componentData, offset, out);
				return true;
			}
			LOG_ERROR("typeid not present in the map!");
			return false;
		}

		static bool Serialize(const size_t typeId, const std::string& name, char* componentData, size_t offset, YAML::Emitter& out)
		{
			if (yaml_table.contains(typeId))
			{
				if (yaml_table[typeId].serialize == nullptr)
					return false;

				yaml_table[typeId].serialize(name, componentData, offset, out);
				return true;
			}
			LOG_ERROR("typeid not present in the map!");
			return false;
		}

		static bool Deserialize(size_t typeId, char* componentData, size_t offset, YAML::Node& value)
		{

			if (yaml_table.contains(typeId))
			{
				if (yaml_table[typeId].deserialize == nullptr)
					return false;
				yaml_table[typeId].deserialize(componentData, offset, value);
				return true;
			}
			LOG_ERROR("typeid not present in the map!");
			return false;
		}
	};

	struct YamlComponentSerializationEntry
	{
		static std::unordered_map<size_t, YamlComponentSerializationEntry> yaml_table;
	private:
		std::function<void(const std::string& name, size_t typeId, char* componentData, YAML::Emitter& out)> serialize{ nullptr };
		std::function<void* (gns::entityHandle entity_handle, YAML::Node& value)> deserialize{ nullptr };

		YamlComponentSerializationEntry(
			std::function<void (const std::string& name, size_t typeId, char* componentData, YAML::Emitter& out)> serializeFn,
			std::function<void* (gns::entityHandle entity_handle, YAML::Node& value)> deserializeFn)
		{
			serialize = serializeFn;
			deserialize = deserializeFn;
		}
	public:
		YamlComponentSerializationEntry() = default;

		template<typename T>
		static void RegisterSerializableComponent(
			std::function<void(const std::string& name, size_t typeId, char* componentData, YAML::Emitter& out)> serializeFn,
			std::function<void* (gns::entityHandle entity_handle, YAML::Node& value)> deserializeFn)
		{
			const size_t typeId = static_cast<size_t>(entt::type_hash<T>::value());
			if (yaml_table.contains(typeId))
			{
				return;
			}
			YamlComponentSerializationEntry e = { serializeFn, deserializeFn };
			yaml_table.emplace(typeId, std::move(e));
		}

		template<typename T>
		static bool Serialize(const std::string& name, char* componentData, YAML::Emitter& out)
		{
			const size_t typeId = static_cast<size_t>(entt::type_hash<T>::value());
			if (yaml_table.contains(typeId))
			{
				if (yaml_table[typeId].serialize == nullptr)
					return false;

				yaml_table[typeId].serialize(name, typeId, componentData, out);
				return true;
			}
			LOG_ERROR("typeid not present in the map!");
			return false;
		}

		static bool Serialize(const size_t typeId, const std::string& name, char* componentData, YAML::Emitter& out)
		{
			if (yaml_table.contains(typeId))
			{
				if (yaml_table[typeId].serialize == nullptr)
					return false;

				yaml_table[typeId].serialize(name, typeId, componentData, out);
				return true;
			}
			LOG_ERROR("typeid not present in the map!");
			return false;
		}

		static void* Deserialize(const size_t typeId, gns::entityHandle entity_handle, YAML::Node& value)
		{
			if (yaml_table.contains(typeId))
			{
				if (yaml_table[typeId].deserialize == nullptr)
					return nullptr;
				return yaml_table[typeId].deserialize(entity_handle, value);
			}
			LOG_ERROR("typeid not present in the map!");
			return nullptr;
		}
	};
}
