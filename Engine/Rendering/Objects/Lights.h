#pragma once
#include "glm/glm.hpp"
#include "../../ECS/ISerializableComponent.h"
#include "typedefs.h"
namespace gns::rendering
{
	struct PointLightComponent : public ISerializeableComponent
	{
		float radius;
		float intensity;

		PointLightComponent() : radius(5.f), intensity(1.5f) {};

		void RegisterFields(ComponentMeta& componentMetaData)
		{
			SET_CMP_NAME(PointLightComponent);
			REGISTER_FIELD(float, radius);
			REGISTER_FIELD(float, intensity);
		}
	};

	struct SpotLightComponent : public ISerializeableComponent
	{
		float distance;
		float intensity;
		float angle;

		void RegisterFields(ComponentMeta& componentMetaData)
		{
			SET_CMP_NAME(SpotLightComponent);
			REGISTER_FIELD(float, distance);
			REGISTER_FIELD(float, intensity);
			REGISTER_FIELD(float, angle);
		}
	};

	struct ColorComponent : public ISerializeableComponent
	{
		color4 color;
		ColorComponent() : color(1.f, 1.f, 1.f, 1.f) {};
		void RegisterFields(ComponentMeta& componentMetaData)
		{
			SET_CMP_NAME(ColorComponent);
			REGISTER_FIELD(color4, color);
		}
	};
}