#include "MenuWindow.h"

void gns::editor::gui::MenuWindow::OnWindowOpen()
{
}

void gns::editor::gui::MenuWindow::OnWindowClosed()
{
}

void gns::editor::gui::MenuWindow::OnWindowDraw()
{
	ImGui::BeginMenuBar();
	ImGui::MenuItem("File");
	ImGui::MenuItem("Window");
	ImGui::MenuItem("Tools");
	ImGui::EndMenuBar();
}

gns::editor::gui::MenuWindow::~MenuWindow() = default;

void gns::editor::gui::MenuWindow::InitWindow()
{
	m_title = "MenuWindow";
	m_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration | ImGuiDockNodeFlags_AutoHideTabBar;
	GuiWindow::InitWindow();
}

bool gns::editor::gui::MenuWindow::OnWindowBegin()
{
	return GuiWindow::OnWindowBegin();
}

void gns::editor::gui::MenuWindow::OnWindowEnd()
{
	GuiWindow::OnWindowEnd();
}
