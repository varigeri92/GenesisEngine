#include "gnspch.h"
#include "SystemsManager.h"

#include "../Utils/Time.h"

std::vector<gns::SystemBase*> gns::SystemsManager::Systems = {};
entt::registry gns::SystemsManager::m_registry = {};

void gns::SystemsManager::UpdateSystems(const float delta_time)
{
	for (size_t i = 0; i < Systems.size(); i++)
	{
		Systems[i]->UpdateSystem(delta_time);
	}
}

void gns::SystemsManager::FixedUpdate(const float fixed_deltaTime)
{
	for (size_t i = 0; i < Systems.size(); i++)
	{
		Systems[i]->FixedUpdate(fixed_deltaTime);
	}
}

void gns::SystemsManager::DisposeSystems()
{
	LOG_INFO("Cleaning up systems...");
	for (size_t i = 0; i < Systems.size(); i++)
	{
		Systems[i]->CleanupSystem();
		delete(Systems[i]);
	}
}

entt::registry& gns::SystemsManager::GetRegistry()
{
	return m_registry;
}
