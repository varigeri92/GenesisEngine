#include "gnspch.h"
#include "SceneManager.h"

#include "../ECS/Component.h"

std::vector<gns::scene::Scene> gns::scene::SceneManager::LoadedScenes = {};
gns::scene::Scene* gns::scene::SceneManager::sActiveScene = nullptr;

gns::scene::Scene& gns::scene::SceneManager::GetActiveScene()
{
	return *sActiveScene;
}

gns::scene::Scene* gns::scene::SceneManager::CreateScene(const std::string& name)
{
	LoadedScenes.emplace_back(Guid::GetNewGuid(), name);
	size_t sceneIndex = LoadedScenes.size() - 1;
	if(sActiveScene == nullptr)
	{
		sActiveScene = &LoadedScenes[sceneIndex];
	}
	Scene* scene = &LoadedScenes[sceneIndex];

	entt::registry& registry = SystemsManager::GetRegistry();
	const entityHandle hidden_root_handle = registry.create();
	registry.emplace<entity::EntityComponent>(hidden_root_handle, "hidden_scene_root");
	registry.emplace<gns::entity::Transform>(hidden_root_handle);
	registry.emplace<gns::entity::Children>(hidden_root_handle);

	Entity h_root_entity = { hidden_root_handle };
	entity::Transform& transform = h_root_entity.GetComponent<entity::Transform>();

	glm::mat4 matrix = glm::mat4(1.f);
	matrix = glm::mat4(1.f);
	matrix = glm::translate(matrix, transform.position);
	matrix = glm::scale(matrix, transform.scale);
	glm::quat quaternion = glm::quat(transform.rotation);
	matrix *= glm::toMat4(quaternion);

	transform.matrix = matrix;


	const entityHandle root_handle = registry.create();
	registry.emplace<entity::EntityComponent>(root_handle, name);
	registry.emplace<gns::entity::Transform>(root_handle);
	registry.emplace<gns::entity::Children>(root_handle);
	registry.emplace<gns::entity::Parent>(root_handle);

	Entity root_entity = { root_handle };
	root_entity.GetComponent<entity::Parent>().parent = hidden_root_handle;

	scene->sceneRootEntity = root_handle;
	return scene;
}

void gns::scene::SceneManager::UnloadScene(const std::string& name)
{
}

void gns::scene::SceneManager::LoadScene(const std::string& path, LoadMode mode)
{
}

void gns::scene::SceneManager::AddEntity(entityHandle entity, Scene* scene)
{
	if(scene == nullptr)
		scene = sActiveScene;
	
	scene->m_entities.emplace_back(entity);
	Entity root = Entity{scene->sceneRootEntity};
	root.Children().emplace_back(entity);
}

void gns::scene::SceneManager::RemoveEntity(entityHandle entity, Scene* scene)
{
}
