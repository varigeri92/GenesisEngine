#include "gnspch.h"
#include "Object.h"

std::unordered_map<gns::guid, gns::Object*> gns::Object::m_objectMap = {};

void gns::Object::ReserveObjectMemory(size_t reserve_size)
{
	m_objectMap.reserve(reserve_size);
}

gns::Object::Object(std::string name) : m_guid(0), name(name)
{
}

void gns::Object::Dispose()
{
	Object::Destroy(m_guid);
}
