#pragma once
#include <string>

#include "imgui.h"

namespace gns::editor::gui::widgets
{
	void ImGuiFloatSlider(const std::string& label, float* value_ptr, float step);
}
