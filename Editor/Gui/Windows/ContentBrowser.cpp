#include "ContentBrowser.h"
#include "Genesis.h"
#include "GenesisFileSystem.h"
#include "GenesisRendering.h"
#include "GenesisSystems.h"
#include "DockSpaceWindow.h"
#include "../../AssetManagement/AssetImporter.h"
#include "../../PathManager.h"
#include "../EditorGuiUtils/SelectionHandler.h"


namespace fs = std::filesystem;
struct spriteSlice
{
	ImVec2 top_left;
	ImVec2 bottom_Right;
};

std::vector<spriteSlice> spriteSheet = {};


void CalculateSprites(uint32_t textureSize, uint32_t sprite_size)
{
	uint32_t rows = textureSize / sprite_size;
	uint32_t columns = textureSize / sprite_size;
	spriteSheet.clear();
	spriteSheet.resize(rows * columns);
	size_t sprite_index = 0;
	for(size_t i = 0; i < columns; i++)
	{
		for (size_t j = 0; j < rows ; j++)
		{
			ImVec2 topleft = {};
			topleft.x = (float)(sprite_size * j) / (float)textureSize;
			topleft.y = (float)(sprite_size * i) / (float)textureSize;

			ImVec2 bottomright = {};
			bottomright.x = (float)(sprite_size * (j + 1)) / (float)textureSize;
			bottomright.y = (float)(sprite_size * (i + 1)) / (float)textureSize;
			spriteSheet[sprite_index] = {topleft, bottomright};
			sprite_index++;
		}
	}
}

std::string GetFileName(std::string const& path)
{
	return path.substr(path.find_last_of("/\\") + 1);
}

void gns::editor::gui::ContentBrowser::OnWindowOpen()
{
}

void gns::editor::gui::ContentBrowser::OnWindowClosed()
{
}

void gns::editor::gui::ContentBrowser::OnWindowDraw()
{
	// Child 1: no border, enable horizontal scrollbar
	{
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
		ImGui::BeginChild("child_dir_tree", ImVec2(ImGui::GetContentRegionAvail().x * 0.25f, ImGui::GetContentRegionAvail().y), 
			directory_tree_flags, window_flags);
		DrawDirectoryStructure(rootEntry);
		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginChild("child_dir_outer", ImGui::GetContentRegionAvail(), directory_view_inner_flags, window_flags);
			if(ImGui::Button("Rescan All"))
			{

				rootEntry.content.clear();
				ScanDirectoryRecursive(PathManager::AssetsPath, rootEntry);
				currentSelectedEntry = &rootEntry;
				contentFileID = 0;
			}
			ImGui::SameLine();
			if (ImGui::Button("Rescan Current"))
			{
				currentSelectedEntry->content.clear();
				ScanDirectoryRecursive(currentSelectedEntry->path, *currentSelectedEntry);
			}
			ImGui::SameLine();


			ImGui::PushItemWidth(300);
			ImGui::SliderFloat("##iconSize", &iconSize, 1, 3, "%.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ImGui::Text(currentSelectedEntry->path.c_str());

			ImGui::BeginChild("child_dir_view", ImGui::GetContentRegionAvail(), directory_view_inner_flags, window_flags);
			DrawContentView(*currentSelectedEntry);
			ImGui::EndChild();
		ImGui::EndChild();

	}
}

gns::editor::gui::ContentBrowser::~ContentBrowser() = default;

gns::editor::gui::ContentBrowser::ContentBrowser()
{
	m_open = true;
	DockSpaceWindow::sWindowMenuMap["contentBrowserWindow"] = &m_open;
	rootEntry = {};
	currentSelectedEntry = nullptr;
	CalculateSprites(2048, 256);
}

gns::RenderSystem* render_system;
gns::rendering::Texture* texture;
void gns::editor::gui::ContentBrowser::InitWindow()
{
	GuiWindow::InitWindow();
	m_title = "Content Browser";
	m_open = true;
	m_flags = 0;
	m_menuPath = "";

	entryMap = {};
	entryMap.reserve(500);
	contentFileID = 0;
	rootEntry = {true, PathManager::AssetsPath, "Assets" };
	rootEntry.entryId = contentFileID;
	ScanDirectoryRecursive(PathManager::AssetsPath, rootEntry);
	currentSelectedEntry = &rootEntry;
	render_system = SystemsManager::GetSystem<RenderSystem>();
	texture = render_system->CreateTexture(PathManager::FromResourcesRelative(R"(EditorResources\Icons.png)"));
}

bool gns::editor::gui::ContentBrowser::OnWindowBegin()
{
	return GuiWindow::OnWindowBegin();
}

void gns::editor::gui::ContentBrowser::OnWindowEnd()
{
	GuiWindow::OnWindowEnd();
}

void gns::editor::gui::ContentBrowser::ScanDirectoryRecursive(const std::string& path, DirectoryEntry& dirEntry)
{
	entryMap[path] = &dirEntry;
	dirEntry.isSelected = false;
	for (const auto& entry : fs::directory_iterator(path))
	{
		if (fileUtils::HasFileExtension(entry.path().string(), "meta"))
		{
			continue;
		}

		contentFileID++;
		std::string fileName = GetFileName(entry.path().string());
		dirEntry.content.emplace_back(entry.is_directory(), entry.path().string(), fileName);
		dirEntry.content[dirEntry.content.size()-1].entryId = contentFileID;
		dirEntry.content[dirEntry.content.size() - 1].assetType = 
			assets::AssetImporter::GetAssetType(fileUtils::GetFileExtension(entry.path().string()));
		if(entry.is_directory())
		{
			ScanDirectoryRecursive(entry.path().string(), dirEntry.content[dirEntry.content.size()-1]);
		}
	}
}



void gns::editor::gui::ContentBrowser::DrawDirectoryStructure(DirectoryEntry& entry)
{
	if (entry.path == "") 
		return;
    for (int i = 0; i < entry.content.size(); i++)
    {
		if(entry.content[i].isDirectory)
		{
			ImGuiTreeNodeFlags flags = base_flags;
			if (entry.content[i].isSelected)
				flags |= ImGuiTreeNodeFlags_Selected;
			if(entry.content[i].content.empty())
			{
				flags |= ImGuiTreeNodeFlags_Leaf;
			}

            ImGui::PushID(ImGui::GetID(entry.content[i].path.c_str()));
			bool node_open = ImGui::TreeNodeEx(entry.content[i].name.c_str(), flags);
			if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
			{
				SelectEntry(entry.content[i]);
			}
            if (node_open)
            {
				DrawDirectoryStructure(entry.content[i]);
				ImGui::TreePop();
            }
            ImGui::PopID();
		}
		else
		{
			ImGui::TreeNodeEx(entry.content[i].name.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
		}
    }
    
}

void gns::editor::gui::ContentBrowser::DrawContentView(DirectoryEntry& entry)
{
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.5f);

	ImGuiStyle& style = ImGui::GetStyle();
	const ImVec2 button_sz(80* iconSize, (80 * iconSize)+ 20);
	const size_t buttons_count = entry.content.size();
	const float window_visible_x2 = ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x;
	for (int i = 0; i < buttons_count; i++)
	{
		ImGuiChildFlags flags = 0;
		if (utils::SelectionHandler::Get() != nullptr)
			if (utils::SelectionHandler::Get()->type == utils::SelectableItemType::Asset
				&& entry.content[i].entryId == utils::SelectionHandler::Get()->itemGuid)
				flags |= ImGuiChildFlags_Borders;

		ImGui::BeginChild(entry.content[i].path.c_str(), button_sz, flags);
		ImGui::PushID(i);
		size_t spriteIndex = 0;
		if(!entry.content[i].isDirectory)
		{
			switch (entry.content[i].assetType)
			{
			case gns::assets::AssetType::None:
				spriteIndex = 1;
				break;
			case gns::assets::AssetType::Mesh:
				spriteIndex = 2;
				break;
			case gns::assets::AssetType::Texture:
				spriteIndex = 4;
				break;
			case gns::assets::AssetType::Sound:
				spriteIndex = 3;
				break;
			case gns::assets::AssetType::Material:
				spriteIndex = 5;
				break;
			case gns::assets::AssetType::Shader:
				spriteIndex = 5;
				break;
			case gns::assets::AssetType::Compute:
				spriteIndex = 5;
				break;
			default:
				spriteIndex = 1;
				break;
			}
		}
		const float inner_button_size = ImGui::GetContentRegionAvail().x;
		auto tex_id = render_system->GetImGuiTexture(texture->handle);
		if (ImGui::ImageButton("##",tex_id,
			{ button_sz.x - 20, button_sz.y - 30 }, spriteSheet[spriteIndex].top_left, spriteSheet[spriteIndex].bottom_Right))
		{
			if (entry.content[i].isDirectory)
			{
				SelectEntry(entry.content[i]);
			}
			else
			{
				gns::editor::utils::SelectionHandler::SetSelectedItem(utils::SelectableItemType::Asset, entry.content[i].path, entry.content[i].entryId);
			}
		}

		if (!entry.content[i].isDirectory)
			if(ImGui::BeginDragDropSource())
			{
				ImGui::SetDragDropPayload("GNS_ASSET", &entry.content[i].path, sizeof(std::string));
				ImGui::EndDragDropSource();
			}

		
		ImGui::PopID();
		ImGui::Text(entry.content[i].name.c_str());
		ImGui::EndChild();
		
		const float last_button_x2 = ImGui::GetItemRectMax().x;
		const float next_button_x2 = last_button_x2 + style.ItemSpacing.x + button_sz.x; // Expected position if next button was on same line
		if (i + 1 < buttons_count && next_button_x2 < window_visible_x2)
			ImGui::SameLine();
	}
	ImGui::PopStyleVar();
}

void gns::editor::gui::ContentBrowser::SelectEntry(DirectoryEntry& entry)
{
	if (currentSelectedEntry != nullptr)
		currentSelectedEntry->isSelected = false;

	entry.isSelected = true;
	currentSelectedEntry = &entry;

	

	LOG_INFO("Selected Directory: " + entry.name);
}
