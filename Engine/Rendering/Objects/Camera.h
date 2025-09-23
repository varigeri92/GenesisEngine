#pragma once
#include <glm/glm.hpp>

namespace gns::rendering
{
	struct Camera
	{
		float m_fov = 60.f;
		float m_near = 0.01f;
		float m_far = 1000.f;
		float m_aspect = (1920.f / 1080.f);
		glm::mat4 m_view;
		glm::mat4 m_projection;
		glm::mat4 m_cameraMatrix;
	};
}