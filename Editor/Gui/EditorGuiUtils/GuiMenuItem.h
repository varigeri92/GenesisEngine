#pragma once
#include <functional>
#include <string>

namespace gns::gui
{
	class GuiWindow;
}

struct GuiMenuItem
{
	std::string path;
	std::string name;
	gns::gui::GuiWindow* m_targetGuiWindow;
	std::function<void> onClickOverride;
};
