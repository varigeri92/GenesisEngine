#pragma once
#include "Utils/VulkanObjects.h"

namespace gns::rendering
{
	class RenderGraph
	{
		std::vector<RenderPass> passes;

		void BeginPass(RenderPass& renderPass);
		void EndPass(RenderPass& renderPass);
	public:
		void AddRenderPass(std::string name, RenderPassData& data, RenderPassContext& ctx,
			std::function<void(RenderPass&)> record);
		void Execute();
};
}
