#include "EditorEntityManager.h"
#include "GenesisSystems.h"
#include "../../Engine/Rendering/Objects/Lights.h"

void gns::editor::entity::EditorEntityManager::CreateNewEntity(PreconfiguredEntityType entityType)
{
	switch (entityType)
	{
	case PreconfiguredEntityType::PointLight:
		{
			Entity entity = Entity::CreateEntity("new Point Light");
			CreatePointLightEmpty(entity);
		}
		break;
	case PreconfiguredEntityType::DirectionalLight:
		{
			Entity entity = Entity::CreateEntity("new Directional Light");
			CreateDirectionalLightEmpty(entity);
		}
		break;
	default: Entity::CreateEntity("new Entity");
		break;
	}
}

void gns::editor::entity::EditorEntityManager::CreatePointLightEmpty(Entity& entity)
{
	entity.AddComponet<gns::rendering::LightComponent>();
	entity.AddComponet<gns::rendering::PointLightComponent>();
	entity.AddComponet<gns::rendering::ColorComponent>();
}

void gns::editor::entity::EditorEntityManager::CreateDirectionalLightEmpty(Entity& entity)
{
	entity.AddComponet<gns::rendering::LightComponent>();
	entity.AddComponet<gns::rendering::ColorComponent>();
}

