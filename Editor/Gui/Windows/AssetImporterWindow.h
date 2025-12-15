#pragma once
#include "GenesisGui.h"

class AssetImporterWindow : public gns::gui::GuiWindow
{
protected:
	void OnWindowOpen() override;
	void OnWindowClosed() override;
	void OnWindowDraw() override;

public:
	~AssetImporterWindow() override;

protected:
	void InitWindow() override;
	bool OnWindowBegin() override;
	void OnWindowEnd() override;

public:
	
};
