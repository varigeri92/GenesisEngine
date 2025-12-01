#pragma once
#include "glm/glm.hpp"
#include "../../Object/Object.h"
#include "../Handles/Handles.h"
#include "../Vulkan/Utils/VulkanObjects.h"

namespace gns::rendering
{
	struct Mesh : public Object
	{
		friend class Object;
		~Mesh() override;
		void Dispose() override;
		bool keepCPU_Data = false;

		std::vector<uint32_t> indices = {};
		std::vector<glm::vec3> positions = {};
		std::vector<glm::vec3> normals = {};
		std::vector<glm::vec2> uvs = {};
		std::vector<glm::vec4> colors = {};
		std::vector<glm::vec3> tangents = {};
		std::vector<glm::vec3> biTangents = {};

		MeshHandle handle;

	protected:
		//Use Object::Create<Mesh>(name) instead!
		Mesh(std::string name);
	};
}