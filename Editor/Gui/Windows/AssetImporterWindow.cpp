#include "AssetImporterWindow.h"

void AssetImporterWindow::OnWindowOpen()
{
}

void AssetImporterWindow::OnWindowClosed()
{
}
ImVec4 warnColor = ImVec4(1, 1, 0, 1);
bool aCheckbox = true;
bool anotherCheckbox = false;
bool anotherCheckbox_II = false;
void AssetImporterWindow::OnWindowDraw()
{
	float label_ratio = 0.80f;
	float available_Width = ImGui::GetContentRegionAvail().x;
	float label_width = available_Width * label_ratio;
	switch (m_assetType) {
	case gns::assets::AssetType::None:
		break;
	case gns::assets::AssetType::Mesh:
		DrawMeshOptions(*meshImportSettings, label_width, available_Width);
		break;
	case gns::assets::AssetType::Texture:
		break;
	case gns::assets::AssetType::Sound:
		break;
	case gns::assets::AssetType::Material:
		break;
	case gns::assets::AssetType::Shader:
		break;
	case gns::assets::AssetType::Compute:
		break;
	}
}

void AssetImporterWindow::DrawMeshOptions(MeshImportSettings& importSettings, float label_width, float available_Width)
{
	ImGui::Text("Asset path");
	ImGui::SeparatorText("Mesh Import Options");
	if (ImGui::BeginTable("assetImporterOptions", 2, ImGuiTableFlags_NoPadInnerX | ImGuiTableFlags_NoPadOuterX))
	{
		ImGui::TableSetupColumn("##", ImGuiTableColumnFlags_WidthFixed, label_width);
		ImGui::TableSetupColumn("##", ImGuiTableColumnFlags_WidthFixed, available_Width - label_width);

		DrawCheckBox(&importSettings.isStatic, "Static:");
		if (importSettings.importSkeleton && importSettings.isStatic)
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::TextColored(warnColor, "WARNING: Mesh can't be static if importing Skeleton");
			ImGui::TableNextRow();
		}
		DrawCheckBox(&importSettings.importMaterials, "Import Materials:");
		DrawCheckBox(&importSettings.importTextures, "Import Textures:");
		DrawCheckBox(&importSettings.importSkeleton, "Import Skeleton:");
		DrawCheckBox(&importSettings.generatePrefab, "Generate Prefab:");


		ImGui::EndTable();
	}
	if (ImGui::Button("Import"))
	{
		if (importSettings.importSkeleton)
		{
			importSettings.isStatic = false;
		}
		m_open = false;
	}
}

void AssetImporterWindow::DrawCheckBox(bool* value, const std::string label)
{
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	ImGui::Text(label.c_str());
	ImGui::TableNextColumn();
	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
	ImGui::Checkbox(("##"+label).c_str(), value);
	ImGui::PopItemWidth();
}

AssetImporterWindow::~AssetImporterWindow() = default;

void AssetImporterWindow::OpenMeshImporterWindow(
	const std::string& path, gns::assets::AssetType type, MeshImportSettings& importSettings)
{
	m_filePath = path;
	m_assetType = type;
	m_open = true;
	meshImportSettings = &importSettings;
}

void AssetImporterWindow::InitWindow()
{
	m_title = "Import Asset";
	m_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse;
	GuiWindow::InitWindow();
	m_open = false;
}

bool AssetImporterWindow::OnWindowBegin()
{
	return GuiWindow::OnWindowBegin();
}

void AssetImporterWindow::OnWindowEnd()
{
	GuiWindow::OnWindowEnd();
}
