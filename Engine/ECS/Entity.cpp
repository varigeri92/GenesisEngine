#include "gnspch.h"
#include "Entity.h"
#include "../ECS/SystemsManager.h"
#include "../ECS/Component.h"
#include "../Scene/Scene.h"
#include "../Scene/SceneManager.h"
using namespace gns::entity;

const std::vector<gns::ComponentData>& gns::Entity::GetAllComponent()
{
    componentsVector.clear();
    for (auto&& curr : SystemsManager::GetRegistry().storage())
    {
        entt::id_type id = curr.first;

        if (auto& storage = curr.second; storage.contains(entity_handle))
        {
            if (ISerializeableComponent::sComponentData.contains((size_t)id))
            {
                void* component_ptr = storage.value(entity_handle);
                componentsVector.emplace_back(component_ptr, id);
            }
        }
    }
    return componentsVector;
}

Transform& gns::Entity::Transform()
{
    return GetComponent<gns::entity::Transform>();
}

std::vector<gns::entityHandle>& gns::Entity::Children()
{
    return GetComponent<gns::entity::Children>().children;
}

void gns::Entity::AddChild(entityHandle handle)
{
    if(handle == entity_handle)
    {
        LOG_ERROR("Entity Cant be its own chidren");
        return;
    }
    Entity entity = { handle };

    Entity parentEntity = { entity.Parent().entity_handle };
    parentEntity.RemoveChild(handle);

    entity.SetParent(entity_handle);
    Children().push_back(handle);
}

void gns::Entity::RemoveChild(entityHandle entity)
{
    size_t index = 0;
    for (size_t i = 0; i<Children().size(); i++)
    {
	    if(entity == Children()[i])
	    {
            index = i;
            break;
	    }
    }
    Children().erase(Children().begin()+index);
}

gns::Entity gns::Entity::Parent()
{
    return Entity{ GetComponent<gns::entity::Parent>().parent };
}

void gns::Entity::SetParent(entityHandle parent)
{
    if(parent == entity_handle)
    {
        LOG_ERROR("Entity cant be its own parent.");
        return;
    }
    GetComponent<gns::entity::Parent>().parent = parent;
}

const std::string& gns::Entity::Name()
{
	return GetComponent<gns::entity::EntityComponent>().name;
}

void gns::Entity::SetName(const std::string& newName)
{
	GetComponent<gns::entity::EntityComponent>().name = newName;
}

gns::guid gns::Entity::GetGuid()
{
    return GetComponent<EntityComponent>().guid;
}

gns::Entity gns::Entity::CreateEntity(const std::string& entityName)
{
    return CreateEntity_Internal(entityName, Guid::GetNewGuid(), &scene::SceneManager::GetActiveScene());
}

gns::Entity gns::Entity::CreateEntity_Internal(const std::string& entityName, const guid guid, gns::scene::Scene* scene)
{
    entt::registry& registry = SystemsManager::GetRegistry();
    entityHandle entity_handle = registry.create();
    registry.emplace<EntityComponent>(entity_handle, entityName, guid);
    registry.emplace<gns::entity::Transform>(entity_handle);
    registry.emplace<gns::entity::Children>(entity_handle);
    registry.emplace<gns::entity::Parent>(entity_handle);
    scene::SceneManager::AddEntity(entity_handle, scene);
    Entity entity(entity_handle);
    entity.SetParent(scene->sceneRootEntity);
    return entity;
}

bool gns::Entity::IsValid() const
{
    return entity_handle != entt::null;
}

void gns::Entity::Delete()
{
    SystemsManager::GetRegistry().destroy(entity_handle);
}

