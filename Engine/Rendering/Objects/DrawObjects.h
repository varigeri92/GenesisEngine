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
			forward(fwd_x, fwd_y, fwd_z, 0), color(cr, cg, cb, intensity) {
		};
		glm::vec4 forward; // .w = unused
		glm::vec4 color; // .w = intensity
	};

	struct SpotLight
	{
		SpotLight() = default;
		SpotLight(float px, float py, float pz, float range, float agle, float intensity,
			float cx, float cy, float cz, float dx, float dy, float dz) :
		position(px, py, pz, range),
		color(cx, cy,cz,intensity),
		direction(dx,dy,dz,agle)
	{}
		glm::vec4 position; // .w = range
		glm::vec4 color; // .w = intensity
		glm::vec4 direction; // .w = angle
	};

	struct SkyLight
	{
		glm::vec4 color{ 1.f, 1.f, 1.f, 1.f};//W->intensity;
		glm::vec4 direction{ 0.f,0.f,0.f,0.f };
	};
}
