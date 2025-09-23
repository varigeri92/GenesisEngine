#pragma once
#include "Texture.h"
#include "../Vulkan/Utils/VulkanObjects.h"

namespace gns::rendering
{
	struct Mesh;
}

namespace gns::rendering
{
	class Shader;

	enum class MaterialFieldType
	{
		_float, _int, vec2, vec3, vec4, color3, color4
	};
	struct MaterialUniformField
	{
		std::string FieldName;
		MaterialFieldType fieldType;
		void* fieldValue_ptr;
	};

	struct MaterialUniformData
	{
		MaterialUniformData() :albedoColor(1, 1, 1, 1), metallic_roughness_AO(0, 0.5, 1, 1) {};
		glm::vec4 albedoColor;
		glm::vec4 metallic_roughness_AO;
	};


	struct Material : public Object
	{
		friend class gns::Object;

		Material() = delete;
		~Material() override;
		void Dispose() override;

		Shader* shader;
		MaterialUniformData uniformData;
		VulkanBuffer* buffer;
		std::vector<Texture*> textures;

		void SetFloat(const std::string& name, float value);
		void SetInt(const std::string& name, uint32_t value);

		void SetVec2(const std::string& name, glm::vec2 value);
		void SetVec3(const std::string& name, glm::vec3 value);
		void SetVec4(const std::string& name, glm::vec4 value);

		void SetColor3(const std::string& name, glm::vec3 value);
		void SetColor4(const std::string& name, glm::vec4 value);

		
	protected:
		// Do not use this constructor! --> use Object::Create<Material>(const std::string& name) instead;
		Material(const std::string& name);
	};
}
