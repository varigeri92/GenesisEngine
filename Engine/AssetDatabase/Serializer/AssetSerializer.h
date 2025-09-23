#pragma once
#include "../../Object/Guid.h"
#include "../../Rendering/Objects/Material.h"

namespace gns::rendering
{
	struct Material;
}

class AssetSerializer
{
private:
	GNS_API void SerializeMaterial(gns::rendering::Material* material);
public:
	template <typename T>
	bool SerializeAsset(T* object_toSerialize)
	{
		bool seriaized = false;
		size_t typeHash = typeid(T).hash_code();
		if(typeHash == typeid(gns::rendering::Material).hash_code())
		{
			SerializeMaterial(object_toSerialize);
			seriaized = true;
		}
		return seriaized;
	};
};
