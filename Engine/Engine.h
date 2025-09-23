#pragma once
#include "API/API.h"
#include "Utils/Logger.h"
#include "ECS/SystemsManager.h"
#include "Window/WindowSystem.h"
#include "ECS/Entity.h"
#include "Object/Guid.h"

namespace gns
{
	class Window;
	class Engine
	{
	public:
		WindowSystem* m_mainWindow;
		GNS_API void InitEngine(const std::string& tittle, const std::string& workingDirectory, const std::string& resourceDirectory);
		GNS_API void OnEngineStart(const std::function<void()>& callback);
		GNS_API void Run();
		GNS_API void ShutDown();
	};
}
