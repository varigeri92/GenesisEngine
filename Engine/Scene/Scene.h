#pragma once
#include "../ECS/Entity.h"
#include "../Object/Guid.h"

namespace gns::scene
{
	struct Scene
	{
	public:
		guid m_guid;
		std::string m_name;
		std::vector<entityHandle> m_entities;
		entityHandle sceneRootEntity;
	};
}

