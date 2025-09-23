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
		Empty, PointLight, DirectionalLight, SkyLight
	};

	class EditorEntityManager
	{
	public:
		void CreateNewEntity(PreconfiguredEntityType entityType);

	private:
		void CreatePointLightEmpty(Entity& entity);
	};
}
