#pragma once
#include "GenesisGui.h"
#include <unordered_map>
#include "../../AssetManagement/AssetLibrary.h"


namespace gns::editor::gui
{
	class ContentBrowser : public gns::gui::GuiWindow
	{
		struct DirectoryEntry
		{
			bool isDirectory;
			std::string path;
			std::string name;
			std::vector<DirectoryEntry> content;
			bool isSelected;
			size_t entryId;
			gns::assets::AssetType assetType;
		};

		std::unordered_map<std::string, DirectoryEntry*> entryMap;

	protected:
		void OnWindowOpen() override;
		void OnWindowClosed() override;
		void OnWindowDraw() override;

	public:
		~ContentBrowser() override;
		ContentBrowser();

	protected:
		void InitWindow() override;
		bool OnWindowBegin() override;
		void OnWindowEnd() override;

	private:



		void ScanDirectoryRecursive(const std::string& path, DirectoryEntry& entry);
		void DrawDirectoryStructure(DirectoryEntry& entry);
		void DrawContentView(DirectoryEntry& entry);
		void SelectEntry(DirectoryEntry& entry);
		DirectoryEntry rootEntry;
		DirectoryEntry* currentSelectedEntry;
		ImTextureID directory_icon = 0;

		ImGuiChildFlags directory_tree_flags = ImGuiChildFlags_ResizeX | ImGuiChildFlags_Borders;
		ImGuiChildFlags directory_view_outer_flags = ImGuiChildFlags_None;
		ImGuiChildFlags directory_view_inner_flags = ImGuiChildFlags_None;

		ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
		float iconSize = 2;
		size_t contentFileID = 0;
	};
}
