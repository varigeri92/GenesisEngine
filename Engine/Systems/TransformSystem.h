#pragma once
#include "../ECS/SystemBase.h"
class TransformSystem : public gns::SystemBase
{
public:
	void InitSystem() override;
	void UpdateSystem(const float deltaTime) override;
	void FixedUpdate(const float fixedDeltaTime) override;

	void CleanupSystem() override;
};
