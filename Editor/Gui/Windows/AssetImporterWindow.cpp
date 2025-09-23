#include "AssetImporterWindow.h"

void AssetImporterWindow::OnWindowOpen()
{
}

void AssetImporterWindow::OnWindowClosed()
{
}

bool aCheckbox = true;
bool anotherCheckbox = false;
bool anotherCheckbox_II = false;
void AssetImporterWindow::OnWindowDraw()
{
	ImGui::Text("Asset path");
	ImGui::SeparatorText("Mesh Import Options");
	float label_ratio = 0.80f;
	float available_Width = ImGui::GetContentRegionAvail().x;
	float label_width = available_Width * label_ratio;
	if (ImGui::BeginTable("assetImporterOptions", 2, ImGuiTableFlags_NoPadInnerX | ImGuiTableFlags_NoPadOuterX))
	{
		ImGui::TableSetupColumn("##", ImGuiTableColumnFlags_WidthFixed, label_width);
		ImGui::TableSetupColumn("##", ImGuiTableColumnFlags_WidthFixed, available_Width - label_width);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("Generate binary asset");
		ImGui::TableNextColumn();
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
		ImGui::Checkbox("##Generate_binary", &aCheckbox);
		ImGui::PopItemWidth();

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("Import as Static");
		ImGui::TableNextColumn();
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
		ImGui::Checkbox("##Import_as_Static", &aCheckbox);
		ImGui::PopItemWidth();

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("Import as Dynamic");
		ImGui::TableNextColumn();
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
		ImGui::Checkbox("##Import_as_Dynamic", &anotherCheckbox);
		ImGui::PopItemWidth();


		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("Import Skeleton");
		ImGui::TableNextColumn();
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
		ImGui::Checkbox("##Import_Skeleton", &anotherCheckbox_II);
		ImGui::PopItemWidth();


		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("Import Materials");
		ImGui::TableNextColumn();
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
		ImGui::Checkbox("##Import_Materials", &anotherCheckbox_II);
		ImGui::PopItemWidth();


		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("Unpack Textures");
		ImGui::TableNextColumn();
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
		ImGui::Checkbox("##Import_Textures", &anotherCheckbox_II);
		ImGui::PopItemWidth();

		ImGui::EndTable();
	}
	if(ImGui::Button("Import"))
	{
		
	}

}

AssetImporterWindow::~AssetImporterWindow() = default;

void AssetImporterWindow::InitWindow()
{
	m_title = "Import Asset";
	m_open = true;
	m_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse;
	GuiWindow::InitWindow();
}

bool AssetImporterWindow::OnWindowBegin()
{
	return GuiWindow::OnWindowBegin();
}

void AssetImporterWindow::OnWindowEnd()
{
	GuiWindow::OnWindowEnd();
}
