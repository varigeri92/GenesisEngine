#pragma once
#include <unordered_map>

#include "GenesisGui.h"
#include "../../../Engine/EventSystem/Event.h"

class AssetImporterWindow : public gns::gui::GuiWindow
{
public:
	struct GenericImportSettings
	{
		bool isStatic = true;
		bool importMaterials = true;
		bool importSkeleton = true;
		bool importTextures = true;
		bool generatePrefab = true;
	};

protected:
	std::unique_ptr<GenericImportSettings> ImportSettings;
	std::string m_filePath;
	gns::assets::AssetType m_assetType {gns::assets::AssetType::Mesh};

	void OnWindowOpen() override;
	void OnWindowClosed() override;
	void OnWindowDraw() override;

	void DrawMeshOptions(float label_width, float available_Width);
	void DrawEmptyOptionsWindow(float label_width, float available_Width);

	void DrawCheckBox(bool* value, const std::string label);

public:
	gns::Event_T<GenericImportSettings> OnImportSettingsEvent;
	~AssetImporterWindow() override;

	void OpenImporterWindow(const std::string& path, gns::assets::AssetType type, GenericImportSettings& importSettings);
protected:
	void InitWindow() override;
	bool OnWindowBegin() override;
	void OnWindowEnd() override;

public:
	
};
