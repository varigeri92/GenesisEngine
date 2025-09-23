#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>

#include "../ECS/ISerializableComponent.h"
#include "../Object/Guid.h"
#include "../Object/Object.h"
#include "../Rendering/Objects/Material.h"

namespace gns::rendering
{
	struct Mesh;
	struct Material;
}

namespace gns::entity
{
	struct EntityComponent : public ISerializeableComponent
	{
		std::string name;
		guid guid;
		bool active = true;

		EntityComponent() = default;
		EntityComponent(std::string entityName) : name(entityName), guid(Guid::GetNewGuid()) {}
		EntityComponent(std::string entityName, gns::guid guid) : name(entityName), guid(guid) {}
		~EntityComponent() override = default;

		EntityComponent(const EntityComponent& other) = delete;
		EntityComponent(EntityComponent&& other) = delete;
		EntityComponent& operator =(EntityComponent&& other) = delete;
		EntityComponent& operator =(const EntityComponent& other) = delete;

		void RegisterFields(ComponentMeta& componentMetaData) override
		{
			SET_CMP_NAME(EntityComponent);

			REGISTER_FIELD(std::string, name);
			REGISTER_FIELD(gns::guid, guid);
			REGISTER_FIELD(bool, active);
		};
	};

	struct SceneComponent : public ISerializeableComponent
	{
		std::string name;
		SceneComponent(std::string name) : name(name) {}
		SceneComponent() = default;

		SceneComponent(const SceneComponent& other) = delete;
		SceneComponent(SceneComponent&& other) = delete;
		SceneComponent& operator =(SceneComponent&& other) = delete;
		SceneComponent& operator =(const SceneComponent& other) = delete;
		~SceneComponent() override = default;

		void RegisterFields(ComponentMeta& componentMetaData) override
		{
			SET_CMP_NAME(SceneComponent);

			REGISTER_FIELD(std::string, name);
		};
	};

	struct Transform : public ISerializeableComponent
	{
		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 scale;
		glm::mat4 matrix;


		Transform()
			: position{ 0,0,0 }, rotation{ 0,0,0 }, scale{ 1,1,1 }
		{}

		Transform(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
			: position{ position }, rotation{ rotation }, scale{ scale }
		{}

		Transform(const Transform& other) = delete;
		Transform(Transform&& other) = delete;
		Transform& operator =(Transform&& other) = delete;
		Transform& operator =(const Transform& other) = delete;
		~Transform() override = default;

		void RegisterFields(ComponentMeta& componentMetaData) override
		{
			SET_CMP_NAME(Transform);

			REGISTER_FIELD(glm::vec3, position);
			REGISTER_FIELD(glm::vec3, rotation);
			REGISTER_FIELD(glm::vec3, scale);
		};

	};

	struct Children
	{
		std::vector<entityHandle> children;
	};

	struct Parent
	{
		entityHandle parent;
	};

	struct MeshComponent : public ISerializeableComponent
	{
		guid meshAsset;
		guid material_ref;

		std::vector<rendering::Material*> materials;
		std::vector<rendering::Mesh*> meshes;

		MeshComponent() = default;
		MeshComponent(guid mesh_ref, guid material_ref) : meshAsset(mesh_ref), material_ref(material_ref) {};
		MeshComponent(guid mesh_ref) : meshAsset(mesh_ref)
		{
			material_ref = Object::Find<gns::rendering::Material>("default_material")->getGuid();
		};
		MeshComponent(const MeshComponent& other) = delete;
		MeshComponent(MeshComponent&& other) = delete;
		MeshComponent& operator =(MeshComponent&& other) = delete;
		MeshComponent& operator =(const MeshComponent& other) = delete;
		~MeshComponent() override = default;

		void RegisterFields(ComponentMeta& componentMetaData) override
		{
			SET_CMP_NAME(MeshComponent);

			REGISTER_FIELD(guid, meshAsset);
		};
	};


}
