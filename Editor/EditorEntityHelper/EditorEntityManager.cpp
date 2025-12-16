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
			CreatePointLight(entity);
		}
		break;
	case PreconfiguredEntityType::DirectionalLight:
		{
			Entity entity = Entity::CreateEntity("new Directional Light");
			CreateDirectionalLight(entity);
		}
		break;
	case PreconfiguredEntityType::SpotLight:
	{
		Entity entity = Entity::CreateEntity("new Spot Light");
		CreateSpotLight(entity);
	}
	case PreconfiguredEntityType::SkyLight:
	{
		Entity entity = Entity::CreateEntity("new Sky Light");
		CreateSkyLight(entity);
	}
	break;
	default: Entity::CreateEntity("new Entity");
		break;
	}
}

void gns::editor::entity::EditorEntityManager::CreatePointLight(Entity& entity)
{
	entity.AddComponet<gns::rendering::LightComponent>();
	entity.AddComponet<gns::rendering::PointLightComponent>();
	entity.AddComponet<gns::rendering::ColorComponent>();
}

void gns::editor::entity::EditorEntityManager::CreateDirectionalLight(Entity& entity)
{
	entity.AddComponet<gns::rendering::LightComponent>();
	entity.AddComponet<gns::rendering::ColorComponent>();
}

void gns::editor::entity::EditorEntityManager::CreateSpotLight(Entity& entity)
{
	entity.AddComponet<gns::rendering::LightComponent>();
	entity.AddComponet<gns::rendering::SpotLightComponent>();
	entity.AddComponet<gns::rendering::ColorComponent>();
}

void gns::editor::entity::EditorEntityManager::CreateSkyLight(Entity& entity)
{
	entity.AddComponet<gns::rendering::LightComponent>();
	entity.AddComponet<gns::rendering::ColorComponent>();
	entity.AddComponet<gns::rendering::SkyComponent>();
}

