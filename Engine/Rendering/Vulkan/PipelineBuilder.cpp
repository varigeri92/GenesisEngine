#include "gnspch.h"
#include "PipelineBuilder.h"
#include "Utils/vkutils.h"
#include "Utils/VulkanObjects.h"
#include "Device.h"
#include "../../Utils/PathHelper.h"
#include "../Utils/FileSystemUtils.h"

namespace gns::rendering
{
	PipelineBuilder::PipelineBuilder(Device* device):m_device(device)
	{ Clear(); }

	PipelineBuilder::~PipelineBuilder()
	{
		vkDestroyShaderModule(m_device->sDevice, vertexModule, nullptr);
		vkDestroyShaderModule(m_device->sDevice, fragmentModule, nullptr);
	}

	void PipelineBuilder::Clear()
	{
	    m_inputAssembly = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };

	    m_rasterizer = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };

	    m_colorBlendAttachment = {};

	    m_multisampling = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };

	    m_pipelineLayout = {};

	    m_depthStencil = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };

	    m_renderInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };

	    m_shaderStages.clear();
	}

	VkPipeline PipelineBuilder::BuildPipeline(VkDevice device)
	{
	    // make viewport state from our stored viewport and scissor.
	    // at the moment we wont support multiple viewports or scissors
	    VkPipelineViewportStateCreateInfo viewportState = {};
	    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	    viewportState.pNext = nullptr;

	    viewportState.viewportCount = 1;
	    viewportState.scissorCount = 1;

	    // setup dummy color blending. We arent using transparent objects yet
	    // the blending is just "no blend", but we do write to the color attachment
	    VkPipelineColorBlendStateCreateInfo colorBlending = {};
	    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	    colorBlending.pNext = nullptr;

	    colorBlending.logicOpEnable = VK_FALSE;
	    colorBlending.logicOp = VK_LOGIC_OP_COPY;
	    colorBlending.attachmentCount = 1;
	    colorBlending.pAttachments = &m_colorBlendAttachment;

	    // completely clear VertexInputStateCreateInfo, as we have no need for it
	    VkPipelineVertexInputStateCreateInfo _vertexInputInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };


	    // build the actual pipeline
	    // we now use all of the info structs we have been writing into into this one
	    // to create the pipeline
	    VkGraphicsPipelineCreateInfo pipelineInfo = { .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
	    // connect the renderInfo to the pNext extension mechanism
	    pipelineInfo.pNext = &m_renderInfo;

	    pipelineInfo.stageCount = (uint32_t)m_shaderStages.size();
	    pipelineInfo.pStages = m_shaderStages.data();
	    pipelineInfo.pVertexInputState = &_vertexInputInfo;
	    pipelineInfo.pInputAssemblyState = &m_inputAssembly;
	    pipelineInfo.pViewportState = &viewportState;
	    pipelineInfo.pRasterizationState = &m_rasterizer;
	    pipelineInfo.pMultisampleState = &m_multisampling;
	    pipelineInfo.pColorBlendState = &colorBlending;
	    pipelineInfo.pDepthStencilState = &m_depthStencil;
	    pipelineInfo.layout = m_pipelineLayout;


	    VkDynamicState state[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	    VkPipelineDynamicStateCreateInfo dynamicInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	    dynamicInfo.pDynamicStates = &state[0];
	    dynamicInfo.dynamicStateCount = 2;

	    pipelineInfo.pDynamicState = &dynamicInfo;

	    VkPipeline newPipeline;
	    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo,
	        nullptr, &newPipeline)
	        != VK_SUCCESS) {
	        LOG_ERROR("Failed To Create Pipeline");
	        return VK_NULL_HANDLE; // failed to create graphics pipeline
	    }
	    else {
	        return newPipeline;
	    }
	}

	void PipelineBuilder::SetShaders(Shader& shader)
	{
		if (!fileUtils::HasFileExtension(shader.fragmentShaderPath, "spv"))
			shader.fragmentShaderPath += ".spv";

		PathHelper::FromResourcesRelative(shader.fragmentShaderPath);
		if (!utils::LoadShaderModule(PathHelper::FromResourcesRelative(shader.fragmentShaderPath).c_str(), Device::sDevice, &fragmentModule)) {
			LOG_ERROR("Error when building the triangle fragment shader module");
		}
		
		if (!fileUtils::HasFileExtension(shader.vertexShaderPath, "spv"))
			shader.vertexShaderPath += ".spv";

		if (!utils::LoadShaderModule(PathHelper::FromResourcesRelative(shader.vertexShaderPath).c_str(), Device::sDevice, &vertexModule)) {
			LOG_ERROR("Error when building the triangle vertex shader module");
		}

		SetShaders(vertexModule, fragmentModule);
	}

	void PipelineBuilder::SetShaders(VkShaderModule vertexShader, VkShaderModule fragmentShader)
	{
	    m_shaderStages.clear();

	    m_shaderStages.push_back(
	        utils::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, vertexShader));

	    m_shaderStages.push_back(
	        utils::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShader));
	}


	void PipelineBuilder::SetInputTopology(VkPrimitiveTopology topology)
	{
	    m_inputAssembly.topology = topology;
	    // we are not going to use primitive restart on the entire tutorial so leave
	    // it on false
	    m_inputAssembly.primitiveRestartEnable = VK_FALSE;
	}

	void PipelineBuilder::SetPolygonMode(VkPolygonMode mode)
	{
		m_rasterizer.polygonMode = mode;
		m_rasterizer.lineWidth = 1.f;
	}

	void PipelineBuilder::SetCullMode(VkCullModeFlags cullMode, VkFrontFace frontFace)
	{
		m_rasterizer.cullMode = cullMode;
		m_rasterizer.frontFace = frontFace;
	}

	void PipelineBuilder::SetMultisampling(bool enable)
	{
		m_multisampling.sampleShadingEnable = VK_FALSE;
		// multisampling defaulted to no multisampling (1 sample per pixel)
		m_multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		m_multisampling.minSampleShading = 1.0f;
		m_multisampling.pSampleMask = nullptr;
		// no alpha to coverage either
		m_multisampling.alphaToCoverageEnable = VK_FALSE;
		m_multisampling.alphaToOneEnable = VK_FALSE;
	}

	void PipelineBuilder::SetBlending(BlendingMode blendingMode)
	{
		switch (blendingMode) {
		case disabled:
			DisableBlending();
			break;
		case additive:
			EnableBlendingAdditive();
			break;
		case alpha:
			EnableBlendingAlphaBlend();
			break;
		default: ;
		}
	}

	void PipelineBuilder::DisableBlending()
	{
		m_colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		m_colorBlendAttachment.blendEnable = VK_FALSE;
	}

	void PipelineBuilder::EnableBlendingAdditive()
	{
		m_colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		m_colorBlendAttachment.blendEnable = VK_TRUE;
		m_colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		m_colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
		m_colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		m_colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		m_colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		m_colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	}

	void PipelineBuilder::EnableBlendingAlphaBlend()
	{
		m_colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		m_colorBlendAttachment.blendEnable = VK_TRUE;
		m_colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		m_colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		m_colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		m_colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		m_colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		m_colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	}

	void PipelineBuilder::SetColorAttachmentFormat(VkFormat format)
	{
		m_colorAttachmentFormat = format;
		// connect the format to the renderInfo  structure
		m_renderInfo.colorAttachmentCount = 1;
		m_renderInfo.pColorAttachmentFormats = &m_colorAttachmentFormat;
	}

	void PipelineBuilder::SetDepthFormat(VkFormat format)
	{
		m_renderInfo.depthAttachmentFormat = format;
	}

	void PipelineBuilder::DisableDepthTest()
	{
		m_depthStencil.depthTestEnable = VK_FALSE;
		m_depthStencil.depthWriteEnable = VK_FALSE;
		m_depthStencil.depthCompareOp = VK_COMPARE_OP_NEVER;
		m_depthStencil.depthBoundsTestEnable = VK_FALSE;
		m_depthStencil.stencilTestEnable = VK_FALSE;
		m_depthStencil.front = {};
		m_depthStencil.back = {};
		m_depthStencil.minDepthBounds = 0.f;
		m_depthStencil.maxDepthBounds = 1.f;
	}

	void PipelineBuilder::EnableDepthTest(bool depthWriteEnable, VkCompareOp op)
	{
		m_depthStencil.depthTestEnable = VK_TRUE;
		m_depthStencil.depthWriteEnable = depthWriteEnable;
		m_depthStencil.depthCompareOp = op;
		m_depthStencil.depthBoundsTestEnable = VK_FALSE;
		m_depthStencil.stencilTestEnable = VK_FALSE;
		m_depthStencil.front = {};
		m_depthStencil.back = {};
		m_depthStencil.minDepthBounds = 0.f;
		m_depthStencil.maxDepthBounds = 1.f;
	}
}
