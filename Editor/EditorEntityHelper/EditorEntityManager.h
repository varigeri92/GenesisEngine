#pragma once
#include "../Gui/EditorGuiUtils/SelectionHandler.h"

namespace gns
{
	struct Entity;
}

namespace gns::editor::entity
{
	enum class PreconfiguredEntityType
	{
		Empty, PointLight, DirectionalLight, SpotLight, SkyLight
	};

	class EditorEntityManager
	{
	public:
		void CreateNewEntity(PreconfiguredEntityType entityType);

	private:
		void CreatePointLight(Entity& entity);
		void CreateDirectionalLight(Entity& entity);
		void CreateSpotLight(Entity& entity);
		void CreateSkyLight(Entity& entity);
	};
}
