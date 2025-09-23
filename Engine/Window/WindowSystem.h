#pragma once
#include "Window.h"
#include "../ECS/SystemBase.h"

class WindowSystem : public gns::SystemBase
{

	gns::Window* m_mainWindiow;
	Screen* m_screen;
public:


	WindowSystem(const std::string& windowTitle,Screen* screen);
	WindowSystem() =  delete;

	void InitSystem() override;
	void UpdateSystem(const float deltaTime) override;
	void FixedUpdate(const float fixedDeltaTime) override;
	void CleanupSystem() override;

	inline bool IsMainWindowOpen()
	{
		return m_mainWindiow->IsOpen();
	}

	gns::Window* GetWindow() const { return m_mainWindiow; };
};
