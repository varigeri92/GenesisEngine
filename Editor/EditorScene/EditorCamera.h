#pragma once
#include "Genesis.h"
#include "GenesisRendering.h"
#include "GenesisSystems.h"

class Screen;

namespace gns::editor::scene
{
	class EditorCamera : public SystemBase
	{
		void SetViewYXZ(glm::vec3 position, glm::vec3 rotation);

	public:
		void InitSystem() override;
		void UpdateSystem(const float deltaTime) override;
		void FixedUpdate(const float fixedDeltaTime) override;
		void CleanupSystem() override;

		rendering::Camera m_camera;
		entity::Transform m_transform;
		float m_cameraSpeed;
		Screen* m_screen;
	private:
		float pitch = 0.0f;
		float yaw = 0.0f;
	};
}
