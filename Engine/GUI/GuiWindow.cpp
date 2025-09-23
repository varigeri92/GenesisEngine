#include "gnspch.h"
#include "GuiWindow.h"

#include "imgui.h"

void gns::gui::GuiWindow::InitWindow(){
	LOG_INFO("Initialize Gui Window: " + m_title);
	m_menuPath = "windows/" + m_title;
	m_open = true;
}

bool gns::gui::GuiWindow::OnWindowBegin()
{
	return ImGui::Begin(m_title.c_str(), &m_open, m_flags);
}

void gns::gui::GuiWindow::OnWindowEnd()
{
	ImGui::End();
}
