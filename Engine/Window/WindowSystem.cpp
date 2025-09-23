#include "gnspch.h"
#include "WindowSystem.h"

WindowSystem::WindowSystem(const std::string& windowTitle, Screen* screen) :m_screen(screen)
{
	m_mainWindiow = new gns::Window();
	m_mainWindiow->InitWindow(windowTitle, screen->width, screen->height);
}

void WindowSystem::InitSystem()
{
}

void WindowSystem::UpdateSystem(const float deltaTime)
{
}

void WindowSystem::FixedUpdate(const float fixedDeltaTime)
{
}

void WindowSystem::CleanupSystem()
{
}
