#pragma once
#include <entt/entt.hpp>
#include "API.h"
#include "SystemBase.h"

namespace gns
{
	class SystemsManager
	{
	public:
		GNS_API static std::vector<SystemBase*> Systems;
	private:
		GNS_API static entt::registry m_registry;
	public:

		static int32_t GetIndexOfType(size_t typeHash)
		{
			for (int32_t i = 0; i < Systems.size(); i++)
			{
				if (Systems[i]->_typeHash == typeHash)
				{
					return i;
				}
			}
			return -1;
		};

		template<typename T, typename... Args>
		static T* RegisterSystem(Args&& ... args)
		{
			size_t typehash = typeid(T).hash_code();
			if (GetIndexOfType(typehash) == -1)
			{
				T* newSystem = new T{ std::forward<Args>(args)... };
				Systems.push_back(newSystem);
				const size_t systemIndex = Systems.size() - 1;
				Systems[systemIndex]->_typeHash = typehash;
				Systems[systemIndex]->InitSystem();
				return newSystem;
			}
			return nullptr;
		};

		static void UpdateSystems(const float delta_time);
		static void FixedUpdate(const float fixed_deltaTime);

		static void DisposeSystems();

		GNS_API static entt::registry& GetRegistry();

		template<typename T, typename... Args>
		static T* FindEntityOfType_Component()
		{
			auto view = GetRegistry().view<T>();
			for (auto [entt, component] : view.each())
			{
				return &component;
			}
			return nullptr;
		}

		template<typename T, typename... Args>
		static entt::entity FindEntityOfType()
		{
			auto view = GetRegistry().view<T>();
			for (auto [entt, component] : view.each())
			{
				return entt;
			}
			return entt::null;
		}

		template<typename T>
		static std::enable_if_t<std::is_base_of_v<SystemBase, T>, void> UnregisterSystem()
		{
			auto typehash = typeid(T).hash_code();
			int32_t index = GetIndexOfType(typehash);
			if (index > -1)
			{
				Systems.erase(Systems.begin() + index);
			}
		}

		template<typename T>
		static std::enable_if_t<std::is_base_of_v<SystemBase, T>, T*> GetSystem()
		{
			auto typehash = typeid(T).hash_code();
			int32_t index = GetIndexOfType(typehash);
			return dynamic_cast<T*>(Systems[index]);
		}
	};

}