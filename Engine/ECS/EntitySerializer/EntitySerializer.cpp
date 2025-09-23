#include "gnspch.h"
#include "EntitySerializer.h"

gns::serialization::EntitySerializer::EntitySerializer(SerializationMode mode) :serializationMode(mode)
{
}

std::vector<size_t> gns::serialization::EntitySerializer::GetSeraizableComponentsOfEntity(Entity& entity)
{
	
	std::vector<ComponentData> Components = entity.GetAllComponent();
	for (ComponentData& component : Components)
	{
		size_t typeId = component.typehash;
		for (const auto & field : ISerializeableComponent::sComponentData[typeId].fields)
		{
			
		}
		
	}
	return{};
}
