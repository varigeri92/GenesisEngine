#pragma once
#include "glm/glm.hpp"

namespace gns::rendering
{
	struct ObjectDrawData
	{
		alignas(16)
		glm::mat4 objectMatrix;
		uint64_t vertexBufferAddress;
		glm::vec3 position;
	};

	struct DrawData
	{
		size_t materialIndex;
		ObjectDrawData objectDrawData;
	};

	struct PointLight
	{
		PointLight() = default;
		PointLight(float px, float py, float pz, float radius, float cr, float cg, float cb, float intensity) :
		position(px, py, pz, radius), color(cr, cg, cb, intensity) {};
		glm::vec4 position; // .w = radius
		glm::vec4 color; // .w = intensity
	};

	struct DirectionalLight
	{
		DirectionalLight() = default;
		DirectionalLight(float fwd_x, float fwd_y, float fwd_z, float cr, float cg, float cb, float intensity) :
			forward(fwd_x, fwd_z, fwd_z, 0), color(cr, cg, cb, intensity) {
		};
		glm::vec4 forward; // .w = unused
		glm::vec4 color; // .w = intensity
	};

	struct SpotLight
	{
		glm::vec4 position; // .w = radius
		glm::vec4 color; // .w = intensity
		glm::vec4 direction; // .w = angle
	};
}
