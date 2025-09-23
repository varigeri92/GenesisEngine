#pragma once
#include <string>
namespace gns
{
	class SystemBase
	{
	public:
		virtual ~SystemBase() = default;
		std::string name;
		size_t _typeHash;

		virtual void InitSystem() = 0;
		virtual void UpdateSystem(const float deltaTime) = 0;
		virtual void FixedUpdate(const float fixedDeltaTime) = 0;
		virtual void CleanupSystem() = 0;
	};
}
