#pragma once
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace gns
{
	struct color4
	{
		float r, g, b, a;
		operator glm::vec4() const { return {r,g,b,a}; }
	};

	struct color3
	{
		float r, g, b;
		operator glm::vec3() const { return { r,g,b }; }

	};

}