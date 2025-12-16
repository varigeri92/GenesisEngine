#pragma once
#include "glm/glm.hpp"
#include "../../ECS/ISerializableComponent.h"
#include "typedefs.h"
namespace gns::rendering
{
	struct PointLightComponent : public ISerializeableComponent
	{
		float radius;

		PointLightComponent() : radius(5.f) {};

		void RegisterFields(ComponentMeta& componentMetaData)
		{
			SET_CMP_NAME(PointLightComponent);
			REGISTER_FIELD(float, radius);
		}
	};

	struct SpotLightComponent : public ISerializeableComponent
	{
		float distance;
		float angle;

		void RegisterFields(ComponentMeta& componentMetaData)
		{
			SET_CMP_NAME(SpotLightComponent);
			REGISTER_FIELD(float, distance);
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

	struct LightComponent : public ISerializeableComponent
	{
		float intensity;
		LightComponent() : intensity(1.f) {};
		void RegisterFields(ComponentMeta& componentMetaData)
		{
			SET_CMP_NAME(LightComponent);
			REGISTER_FIELD(float, intensity);
		}
	};

	struct SkyComponent : public ISerializeableComponent
	{
		guid hdr { 0 };
		void RegisterFields(ComponentMeta& componentMetaData)
		{
			SET_CMP_NAME(SkyComponent);
			REGISTER_FIELD(guid, hdr);
		}
	};
}