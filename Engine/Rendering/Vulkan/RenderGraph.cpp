#include "gnspch.h"
#include "RenderGraph.h"

void gns::rendering::RenderGraph::BeginPass(RenderPass& renderPass)
{
}

void gns::rendering::RenderGraph::EndPass(RenderPass& renderPass)
{
}

void gns::rendering::RenderGraph::AddRenderPass(std::string name, RenderPassData& data, RenderPassContext& ctx, std::function<void(RenderPass&)> record)
{
	RenderPass renderPass = {
		.name = name,
		.data = std::move(data),
		.ctx = std::move(ctx),
		.record = std::move(record)
	};

	passes.push_back(renderPass);
}

void gns::rendering::RenderGraph::Execute()
{
	for (size_t i = 0; i < passes.size(), i++;)
	{
		BeginPass(passes[i]);
		passes[i].record(passes[i]);
		EndPass(passes[i]);
	}
}
