#pragma once
#include "Entity.h"
#include <typeinfo>
#include "../Object/Guid.h"
#include "../Utils/Logger.h"


#define SET_CMP_NAME(cmp_name) typedef cmp_name component

#define REGISTER_COMPONENT(type) ISerializeableComponent::RegisterComponent<type>(#type, sizeof(type), (size_t)entt::type_hash<type>::value())
#define REGISTER_FIELD(type, name) componentMetaData.fields.emplace_back(#name, sizeof(type), typeid(type).hash_code(), offsetof(component, name))

namespace gns
{
	
struct FieldMetadata
{
	std::string name;
	size_t size = sizeof(std::string);
	size_t type = typeid(std::string).hash_code();
	size_t offset;
};

struct ComponentMeta
{
	std::string name;
	size_t size = sizeof(std::string);
	size_t type = typeid(std::string).hash_code();
	std::vector<FieldMetadata> fields;
};
struct ISerializeableComponent
{
	GNS_API static std::unordered_map<size_t, ComponentMeta> sComponentData;

	virtual ~ISerializeableComponent() = default;

	virtual void RegisterFields(ComponentMeta& componentMetaData) = 0;

	template <typename IComponent_t, typename = std::enable_if<std::is_base_of_v<ISerializeableComponent, IComponent_t>>::type>
	static void RegisterComponent(const std::string& name, size_t size, size_t typeId)
	{
		ComponentMeta newMeta = { name, size, typeId, {} };
		size_t componentTypeId = (size_t)entt::type_hash<IComponent_t>::value();
		sComponentData[componentTypeId] = std::move(newMeta);
		IComponent_t comp;
		comp.RegisterFields(sComponentData[componentTypeId]);
	}
};
}