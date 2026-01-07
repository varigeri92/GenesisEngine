#include "gnspch.h"
#include "Object.h"

std::unordered_map<gns::guid, gns::Object*> gns::Object::m_objectMap = {};
std::unordered_map<gns::guid, gns::Object*> gns::Object::m_intermediateObjectMap = {};

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

template <typename T, typename, typename ... Args>
T* gns::Object::CreateIntermediate(guid _guid, Args&&... args)
{
	m_intermediateObjectMap[_guid] = new T(std::forward<Args>(args)...);
	m_intermediateObjectMap[_guid]->m_guid = _guid;
	return dynamic_cast<T*>(m_intermediateObjectMap[_guid]);
}

void gns::Object::FinalizeIntermediate(guid _guid)
{
	m_objectMap[_guid] = m_intermediateObjectMap[_guid];
	m_intermediateObjectMap.erase(_guid);
}
