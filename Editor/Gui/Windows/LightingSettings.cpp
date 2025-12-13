#include "LightingSettings.h"
#include "Genesis.h"
#include "GenesisRendering.h"
#include "GenesisSystems.h"
#include "imgui.h"
#include "../../../Engine/ECS/SystemsManager.h"


namespace gns::editor
{
	static ImGuiTableFlags table_flags = ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_SizingFixedFit
		| ImGuiTableFlags_NoPadInnerX | ImGuiTableFlags_NoPadOuterX;


	void DrawFloatField(const std::string& label, float* value, float steps)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text(label.c_str());
		ImGui::TableNextColumn();
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
		ImGui::DragFloat(("##" + label).c_str(), value, steps);
		ImGui::PopItemWidth();
	}

	LightingSettings::LightingSettings() : gui::GuiWindow()
	{
		m_title = "LightingSettings";
		m_open = true;
	}

	void LightingSettings::OnWindowOpen()
	{
	}

	void LightingSettings::OnWindowClosed()
	{
	}

	float testValue = 0.f;
	void LightingSettings::OnWindowDraw()
	{
		float label_ratio = 0.35f;
		float available_Width = ImGui::GetContentRegionAvail().x;
		float label_width = available_Width * label_ratio;

		if (ImGui::BeginTable("SceneLighting", 2, table_flags))
		{
			ImGui::TableSetupColumn("##", ImGuiTableColumnFlags_WidthFixed, label_width);
			ImGui::TableSetupColumn("##", ImGuiTableColumnFlags_WidthFixed, available_Width - label_width);
			DrawFloatField("Normal Offset", &m_renderSystem->GetLightningSettings()->normalOffset, 0.0001f);
			DrawFloatField("Shadow Bias", &m_renderSystem->GetLightningSettings()->shadowBias, 0.0001f);
			DrawFloatField("Slope Scale", &m_renderSystem->GetLightningSettings()->slopeScale, 0.0001f);
			DrawFloatField("Half Extent", &m_renderSystem->GetLightningSettings()->halfExtent, 0.1f);
			DrawFloatField("Near Plane", &m_renderSystem->GetLightningSettings()->nearPlane, 1.f);
			DrawFloatField("Far Plane", &m_renderSystem->GetLightningSettings()->farPlane, 1.f);
			ImGui::EndTable();
		}
	}

	LightingSettings::~LightingSettings()
	{
	}

	void LightingSettings::InitWindow()
	{
		m_renderSystem = gns::SystemsManager::GetSystem<gns::RenderSystem>();
		GuiWindow::InitWindow();
	}

	bool LightingSettings::OnWindowBegin()
	{
		return GuiWindow::OnWindowBegin();
	}

	void LightingSettings::OnWindowEnd()
	{
		GuiWindow::OnWindowEnd();
	}
}
