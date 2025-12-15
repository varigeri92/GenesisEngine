#pragma once
#include "GenesisGui.h"
#include <unordered_map>

class DockSpaceWindow : public  gns::gui::GuiWindow
{
	void OnWindowOpen() override;
	void OnWindowClosed() override;
	void OnWindowDraw() override;

public:
	~DockSpaceWindow() override;
	static std::unordered_map<std::string, bool*> sWindowMenuMap;
	static void AddWindowToMenu(const std::string& windowMenuPath, bool* isOpenFlag);
protected:
	void InitWindow() override;
	bool OnWindowBegin() override;
	void OnWindowEnd() override;
};
