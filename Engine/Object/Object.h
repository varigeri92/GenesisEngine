#pragma once
#include <unordered_map>

#include "Guid.h"
#include "../Utils/Logger.h"

namespace gns
{
	class Object
	{
	public:
		GNS_API static std::unordered_map<guid, Object*> m_objectMap;

		static void ReserveObjectMemory(const size_t reserve_size);

		Object(std::string name);
		virtual ~Object() = default;

		guid m_guid;
		std::string name;

		inline guid getGuid() const { return m_guid; };

		template<typename T, typename = std::enable_if<std::is_base_of<Object, T>::value>::type, typename... Args>
		static T* Create(Args&& ... args)
		{
			guid guid = Guid::GetNewGuid();
			m_objectMap[guid] = new T(std::forward<Args>(args)...);
			m_objectMap[guid]->m_guid = guid;
			return dynamic_cast<T*>(m_objectMap[guid]);
		}

		template<typename T, typename = std::enable_if<std::is_base_of<Object, T>::value>::type, typename... Args>
		static T* CreateWithGuid(guid _guid, Args&& ... args)
		{
			if(m_objectMap.contains(_guid))
			{
				LOG_INFO("object map contains guid: " + std::to_string(_guid));
				return dynamic_cast<T*>(m_objectMap[_guid]);
			}
			m_objectMap[_guid] = new T(std::forward<Args>(args)...);
			m_objectMap[_guid]->m_guid = _guid;
			return dynamic_cast<T*>(m_objectMap[_guid]);
		}

		template<typename T, typename = std::enable_if<std::is_base_of<Object, T>::value>::type, typename... Args>
		static T* Get(guid guid)
		{
			return dynamic_cast<T*>(m_objectMap[guid]);
		}

		template<typename T, typename = std::enable_if<std::is_base_of<Object, T>::value>::type, typename... Args>
		static T* Find(const std::string& name)
		{
			for (auto element : m_objectMap)
			{
				if (element.second->name == name)
					return dynamic_cast<T*>(element.second);
			}
			return nullptr;
		}

		virtual void Dispose();
	};
}

