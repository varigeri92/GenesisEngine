#pragma once
#include "entt/entt.hpp"
#include "../ECS/SystemsManager.h"
#include "../Object/Guid.h"


namespace gns
{
	namespace scene
	{
		struct Scene;
	}

	namespace entity
	{
		struct Children;
		struct Transform;
	}

	typedef entt::entity entityHandle;

	struct ComponentData
	{
		void* data;
		size_t typehash;
	};


	struct Entity
	{
		entt::entity entity_handle;
		Entity(entt::entity entity_handle) :
			entity_handle{ entity_handle }
		{
		};
		Entity() : entity_handle{ entt::null } {};

		Entity& operator=(const Entity& other)
		{
			entity_handle = other.entity_handle;
			return *this;
		}

		operator entityHandle() const
		{
			return entity_handle;
		}
		GNS_API bool IsValid() const;
		operator bool() const { return IsValid(); }

		GNS_API void Delete();

		template<typename T, typename... Args>
		T& AddComponet(Args&& ... args)
		{
			T& component = SystemsManager::GetRegistry().emplace<T>(entity_handle, std::forward<Args>(args)...);
			return component;
		}



		template<typename T>
		T& GetComponent()
		{
			return SystemsManager::GetRegistry().get<T>(entity_handle);
		}

		template<typename T>
		bool TryGetComponent(T* component)
		{
			component = SystemsManager::GetRegistry().try_get<T>(entity_handle);
			return component != nullptr;
		}

		GNS_API const std::vector<gns::ComponentData>& GetAllComponent();

		GNS_API entity::Transform& Transform();
		GNS_API std::vector<gns::entityHandle>& Children();
		GNS_API void AddChild(entityHandle entity_handle);
		GNS_API void RemoveChild(entityHandle entity);
		GNS_API Entity Parent();
		GNS_API const std::string& Name();
		GNS_API void SetName(const std::string& newName);
		GNS_API guid GetGuid();

		GNS_API static Entity CreateEntity(const std::string& entityName);
		static Entity CreateEntity_Internal(const std::string& entityName, const guid guid, gns::scene::Scene* scene);
	private:
		GNS_API void SetParent(entityHandle parent);
		std::vector<gns::ComponentData> componentsVector;
	};
}
