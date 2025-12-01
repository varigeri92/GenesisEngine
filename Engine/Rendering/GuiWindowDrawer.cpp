#include "gnspch.h"
#include "GuiWindowDrawer.h"

#include "imgui.h"
#include "../GUI/GuiWindow.h"

std::vector<gns::gui::GuiWindow*> gns::GuiWindowDrawer::GuiWindows = {};
bool show_imgui_demo_window = false;
void gns::GuiWindowDrawer::DrawWindows()
{

	for (auto guiWindow : GuiWindows)
	{
		if(!guiWindow->m_open)
			continue;
		if(guiWindow->OnWindowBegin())
		{
			guiWindow->OnWindowDraw();
		}
		guiWindow->OnWindowEnd();
	}
	if(show_imgui_demo_window)
		ImGui::ShowDemoWindow();
}
