#pragma once
#include <string>

#include "API.h"
#include "../../ECS/Entity.h"

namespace gns::scene
{
	struct Scene;
}

namespace gns::serialization
{
	class SceneSerializer
	{
	public:
		SceneSerializer() = default;
		~SceneSerializer() = default;
		GNS_API bool SaveScene(const std::string& outFilePath);
		GNS_API std::string SerializeScene(scene::Scene* scene);
		GNS_API scene::Scene* DeserializeScene(const std::string& sceneFilePath);

		static void RegisterTable();

	private:
		void ProcessSceneReferences(gns::scene::Scene* scene);
	};
}
