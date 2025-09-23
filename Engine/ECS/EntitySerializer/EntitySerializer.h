#pragma once
#include "../Component.h"
#include "../Entity.h"
#include "../ISerializableComponent.h"

namespace gns::serialization
{

class EntitySerializer
{
public:
	enum class SerializationMode
	{
		binary, text
	};

	SerializationMode serializationMode;

	GNS_API EntitySerializer(SerializationMode mode);

	void DeSerializeEntity(const std::string& filePathOrPackageAddress);

	GNS_API std::vector<size_t> GetSeraizableComponentsOfEntity(Entity& entity);
};
}

