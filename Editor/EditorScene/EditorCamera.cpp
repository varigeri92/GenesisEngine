#include "EditorCamera.h"
#include "../../Engine/Input/InputBackend.h"
#include "../../Engine/Rendering/RenderSystem.h"
#include "../../Engine/ECS/SystemsManager.h"
#include "../../Engine/Utils/Logger.h"
#include "../../Engine/Window/Screen.h"
#include "SDL2/SDL_keycode.h"

void gns::editor::scene::EditorCamera::InitSystem()
{
	m_transform.position = { 0,0,8 };
	m_camera = { 60.f, 0.001f, 1000.f };
	m_cameraSpeed = 5;
	RenderSystem* renderSystem = SystemsManager::GetSystem<RenderSystem>();
	renderSystem->SetActiveCamera(&m_camera, &m_transform);
	m_screen = renderSystem->GetTargetScren();
}

void gns::editor::scene::EditorCamera::SetViewYXZ(glm::vec3 position, glm::vec3 rotation)
{
	const float c3 = glm::cos(rotation.z);
	const float s3 = glm::sin(rotation.z);
	const float c2 = glm::cos(rotation.x);
	const float s2 = glm::sin(rotation.x);
	const float c1 = glm::cos(rotation.y);
	const float s1 = glm::sin(rotation.y);
	const glm::vec3 u{ (c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1) };
	const glm::vec3 v{ (c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3) };
	const glm::vec3 w{ (c2 * s1), (-s2), (c1 * c2) };
	glm::mat4 view = glm::mat4{ 1.f };
	view[0][0] = u.x;
	view[1][0] = u.y;
	view[2][0] = u.z;
	view[0][1] = v.x;
	view[1][1] = v.y;
	view[2][1] = v.z;
	view[0][2] = w.x;
	view[1][2] = w.y;
	view[2][2] = w.z;
	view[3][0] = -glm::dot(u, position);
	view[3][1] = -glm::dot(v, position);
	view[3][2] = -glm::dot(w, position);
	m_camera.m_view = view;
}

void gns::editor::scene::EditorCamera::UpdateSystem(const float deltaTime)
{
	m_camera.m_aspect = m_screen->aspectRatio;

	if (gns::InputBackend::GetMouseButton(3))
	{
		float speed = (deltaTime * m_cameraSpeed);
		float mouse_sensitivity = (deltaTime * 1.f);
		m_transform.rotation.x += -gns::InputBackend::GetMouseVelocity().y * mouse_sensitivity;
		m_transform.rotation.y += -gns::InputBackend::GetMouseVelocity().x * mouse_sensitivity;

		glm::quat rotation = { m_transform.rotation };
		float pitch = (m_transform.rotation.x); // Rotation around the X-axis
		float yaw = (m_transform.rotation.y);   // Rotation around the Y-axis
		float roll = (m_transform.rotation.z); // Rotation around the Z-axis

		glm::vec3 camFront;
		camFront.x = cos(pitch) * sin(yaw);
		camFront.y = sin(pitch);
		camFront.z = cos(pitch) * cos(yaw);
		camFront = glm::normalize(camFront);
		camFront.y *= -1;
		camFront *= -1;

		glm::vec3 worldUp = glm::vec3(0.0f, -1.0f, 0.0f);
		glm::vec3 camRight = -glm::normalize(glm::cross(worldUp, camFront));
		glm::vec3 camUp = glm::normalize(glm::cross(camFront, camRight));
		//fwd:
		if (gns::InputBackend::GetKey(SDLK_w)) {
			m_transform.position += camFront * speed;
		}
		//back:
		if (gns::InputBackend::GetKey(SDLK_s)) {
			m_transform.position -= camFront * speed;
		}
		//left:
		if (gns::InputBackend::GetKey(SDLK_a)) {
			m_transform.position += camRight * speed;
		}
		//right:
		if (gns::InputBackend::GetKey(SDLK_d)) {
			m_transform.position -= camRight * speed;
		}

		//down:
		if (gns::InputBackend::GetKey(SDLK_q)) {
			m_transform.position -= camUp * speed;
		}
		//up:
		if (gns::InputBackend::GetKey(SDLK_e)) {
			m_transform.position += camUp * speed;
		}


	}
	//m_camera.view = m_transform.matrix;
	m_camera.m_projection = glm::perspective(
		glm::radians(m_camera.m_fov), (m_camera.m_aspect), m_camera.m_near, m_camera.m_far);
	m_camera.m_projection[1][1] *= -1;
	SetViewYXZ(m_transform.position, m_transform.rotation);
	m_camera.m_cameraMatrix = m_camera.m_projection * (m_camera.m_view);
}

void gns::editor::scene::EditorCamera::FixedUpdate(const float fixedDeltaTime)
{

}

void gns::editor::scene::EditorCamera::CleanupSystem()
{
}
