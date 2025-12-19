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

		static Scene& GetActiveScene();
		static Scene* CreateScene(const std::string& name);
		static void SetActiveScene(Scene* scene);
		static void UnloadScene(const std::string& name);
		static void LoadScene(const std::string& path, LoadMode mode = LoadMode::Additive);

	private:
		static void AddEntity(entityHandle entity, Scene* scene = nullptr);
		static void RemoveEntity(entityHandle entity, Scene* scene = nullptr);
	};
}
