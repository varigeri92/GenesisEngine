#include "Widgets.h"

void gns::editor::gui::widgets::ImGuiFloatSlider(const std::string& label, float* value_ptr, float step)
{
	float label_ratio = 0.2f;
	float available_Width = ImGui::GetContentRegionAvail().x;
	float label_width = available_Width * label_ratio;
	if(ImGui::BeginTable("table_hello!", 2, ImGuiTableFlags_NoPadInnerX | ImGuiTableFlags_NoPadOuterX))
	{
		ImGui::TableSetupColumn("##", ImGuiTableColumnFlags_WidthFixed, label_width);
		ImGui::TableSetupColumn("##", ImGuiTableColumnFlags_WidthFixed, available_Width-label_width);
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text(label.c_str());
		ImGui::TableNextColumn();
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
		ImGui::DragFloat(("##"+label).c_str(), value_ptr, step, 0, 1);
		ImGui::PopItemWidth();
		ImGui::EndTable();
	}
}
