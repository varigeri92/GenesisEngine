#include "gnspch.h"
#include "TransformSystem.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/glm.hpp>

#include "../ECS/Component.h"
#include "../Scene/SceneManager.h"

void TransformSystem::InitSystem()
{
}

void UpdateTransform(gns::Entity& entity)
{
	const gns::entity::Transform& parent_transform = entity.Parent().Transform();

    gns::entity::Transform& transform = entity.Transform();
    glm::mat4 matrix = glm::mat4(1.f);
    matrix = glm::mat4(1.f);
    matrix = glm::translate(matrix, transform.position);
    matrix = glm::scale(matrix, transform.scale);
    glm::quat quaternion = glm::quat(transform.rotation);
    matrix *= glm::toMat4(quaternion);

    transform.matrix = parent_transform.matrix * matrix;

	for (gns::entityHandle child : entity.Children())
	{
		gns::Entity child_entity = { child };
        UpdateTransform(child_entity);
	}
}

void TransformSystem::UpdateSystem(const float deltaTime)
{
    /*
    auto object_view = gns::SystemsManager::GetRegistry()
        .view<gns::entity::EntityComponent, gns::entity::Transform, gns::entity::Parent>();
    for (auto [entt, entity, transform, parent] : object_view.each())
    {
        if (!entity.active)
            continue;

        glm::mat4 matrix = glm::mat4(1.f);
        matrix = glm::mat4(1.f);
        matrix = glm::translate(matrix, transform.position);
        matrix = glm::scale(matrix, transform.scale);
        glm::quat quaternion = glm::quat(transform.rotation);
        matrix *= glm::toMat4(quaternion);
        transform.matrix = matrix;

        gns::Entity parentEntity = gns::Entity{parent.parent};
        parentEntity.GetComponent<gns::entity::Transform>().matrix;
    }
    */

    for (gns::scene::Scene& loadedScene : gns::scene::SceneManager::LoadedScenes)
    {
        gns::Entity entity = { loadedScene.sceneRootEntity };
        UpdateTransform(entity);
    }
}

void TransformSystem::FixedUpdate(const float fixedDeltaTime)
{
}

void TransformSystem::CleanupSystem()
{
}
