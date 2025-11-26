#include "gnspch.h"
#include "Engine.h"
#include "AssetDatabase/AssetLoader.h"
#include "Utils/Time.h"
#include "Window/Window.h"
#include "Input/InputBackend.h"
#include "Rendering/Vulkan/Device.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/Objects/Lights.h"
#include "Scene/SceneManager.h"
#include "Window/Screen.h"
#include "Window/WindowSystem.h"
#include "Object/Object.h"
#include "Rendering/Objects/Mesh.h"
#include "Systems/TransformSystem.h"
#include "Utils/PathHelper.h"

#define FIXED_UPDATE_RATE 30.f

namespace gns
{
	constexpr size_t RESERVE_OBJECT_COUNT = 500;
	std::unordered_map<size_t, ComponentMeta> ISerializeableComponent::sComponentData = {};

	Screen* defaultScreen;
	void Engine::InitEngine(const std::string& tittle, const std::string& workingDirectory, const std::string& resourceDirectory)
	{
		PathHelper::AssetsPath = workingDirectory;
		PathHelper::ResourcesPath = resourceDirectory;

		REGISTER_COMPONENT(entity::EntityComponent);
		REGISTER_COMPONENT(entity::SceneComponent);
		REGISTER_COMPONENT(entity::Transform);
		REGISTER_COMPONENT(entity::MeshComponent);
		REGISTER_COMPONENT(rendering::PointLightComponent);
		REGISTER_COMPONENT(rendering::SpotLightComponent);
		REGISTER_COMPONENT(rendering::ColorComponent);

		defaultScreen = new Screen{1920, 1080, 1.f, ((float)1920/(float)1080), false, true};
		m_mainWindow = SystemsManager::RegisterSystem<WindowSystem>(tittle, defaultScreen);
		SystemsManager::RegisterSystem<TransformSystem>();
	}

	void Engine::OnEngineStart(const std::function<void()>& callback)
	{
		auto* renderSystem = SystemsManager::RegisterSystem<RenderSystem>(defaultScreen);

		scene::SceneManager::CreateScene("Sponza_Scene");
		callback();
	}

	void Engine::Run()
	{
		float fixedUpdateTime = 0;
		while (m_mainWindow->GetWindow()->IsOpen())
		{
			Time::StartFrameTime();
			InputBackend::ProcessInput(m_mainWindow->GetWindow()->sdl_event, m_mainWindow->GetWindow());

			const float delta_time = Time::DeltaTime();
			fixedUpdateTime += delta_time;

			SystemsManager::UpdateSystems(delta_time);
			if(fixedUpdateTime >= 1.f / FIXED_UPDATE_RATE)
			{
				SystemsManager::FixedUpdate(1.f /FIXED_UPDATE_RATE);
				fixedUpdateTime = 0;
			}
			Time::EndFrameTime();
		}
	}

	void Engine::ShutDown()
	{
		m_mainWindow->GetWindow()->CloseWindow();
		SystemsManager::DisposeSystems();
		delete(defaultScreen);
	}
}
