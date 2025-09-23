#pragma once
#include <glm/glm.hpp>
#include "yaml-cpp/yaml.h"

namespace YAML
{
	inline Emitter& operator<<(Emitter& emitter, const glm::vec4& v) {
		emitter << YAML::Flow;
		emitter << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return emitter;
	}

	inline Emitter& operator<<(Emitter& emitter, const glm::vec3& v) {
		emitter << YAML::Flow;
		emitter << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return emitter;
	}

	inline Emitter& operator<<(Emitter& emitter, const glm::vec2& v) {
		emitter << YAML::Flow;
		emitter << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return emitter;
	}
}