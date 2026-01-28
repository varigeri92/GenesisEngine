#pragma once
#include "Scene.h"

namespace gns::scene
{

	class SceneManager
	{
		friend struct Entity;

		static Scene* sActiveScene;
	public:
		GNS_API static std::vector<Scene> LoadedScenes;

		enum LoadMode
		{
			Additive, UnloadCurrent
		};

		GNS_API static Scene& GetActiveScene();
		GNS_API static Scene* CreateScene(const std::string& name);
		GNS_API static void SetActiveScene(Scene* scene);
		GNS_API static void UnloadScene(const std::string& name);
		GNS_API static void UnloadActiveScene();
		GNS_API static void LoadScene(const std::string& path, LoadMode mode = LoadMode::Additive);
	private:
		static void AddEntity(entityHandle entity, Scene* scene = nullptr);
		static void RemoveEntity(entityHandle entity, Scene* scene = nullptr);
	};
}
