#pragma once
#include <unordered_map>

#include "Guid.h"
#include "../Utils/Logger.h"

namespace gns
{
	class Object
	{
		guid m_guid; //objectInstance GUID
		guid m_assetHandle {0}; //AssetReference - AssetSource GUID 
	public:
		GNS_API static std::unordered_map<guid, Object*> m_objectMap;
		static void ReserveObjectMemory(const size_t reserve_size);
		Object(std::string name);
		virtual ~Object() = default;

		std::string name;
		inline guid getGuid() const { return m_guid; }
		inline guid getAssetHandle() const { return m_guid; }
		inline void SetAssetHandle(gns::guid asset_handle) {m_assetHandle = asset_handle;}

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
				T* existing = dynamic_cast<T*>(m_objectMap[_guid]);
				if (!existing) {
					LOG_ERROR("GUID collision: " + std::to_string(_guid) +
						" exists but is not of type " + typeid(T).name());
					return nullptr;
				}
				return existing;
			}
			m_objectMap[_guid] = new T(std::forward<Args>(args)...);
			m_objectMap[_guid]->m_guid = _guid;
			return dynamic_cast<T*>(m_objectMap[_guid]);
		}

		template<typename T, typename = std::enable_if<std::is_base_of<Object, T>::value>::type, typename... Args>
		static T* Get(guid guid)
		{
			if (m_objectMap.contains(guid))
				return dynamic_cast<T*>(m_objectMap[guid]);
			else
				return nullptr;
		}

		template<typename T, typename = std::enable_if<std::is_base_of<Object, T>::value>::type, typename... Args>
		static T* Find(const std::string& name)
		{
			for (const auto& element : m_objectMap)
			{
				if (element.second->name == name)
					return dynamic_cast<T*>(element.second);
			}
			return nullptr;
		}


		static void Destroy(guid id)
		{
			auto it = m_objectMap.find(id);
			if (it == m_objectMap.end()) return;

			Object* obj = it->second;
			m_objectMap.erase(it);
			delete obj;
		}

		static void ClearAll()
		{
			for (auto& it : m_objectMap)
				delete it.second;
			m_objectMap.clear();
		}

		virtual void Dispose();
	};
}

