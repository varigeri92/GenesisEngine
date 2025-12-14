#include "DockSpaceWindow.h"
#include "Genesis.h"
#include "imgui.h"
#include "GenesisSerialization.h"
#include "../../PathManager.h"


std::unordered_map<std::string, bool*> DockSpaceWindow::sWindowMenuMap = {};
static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

void DockSpaceWindow::AddWindowToMenu(const std::string& windowMenuPath, bool* isOpenFlag)
{

}

void DockSpaceWindow::OnWindowOpen()
{
}

void DockSpaceWindow::OnWindowClosed()
{
}

void DockSpaceWindow::OnWindowDraw()
{

	ImGuiID dockspace_id = ImGui::GetID("DockSpace");
	ImGui::DockSpaceOverViewport(dockspace_id);
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Save", "(Ctrl+S)"))
			{
				gns::serialization::SceneSerializer serializer;
				serializer.SaveScene(PathManager::FromAssetsRelative("scene.gnsscene"));
				LOG_INFO("Save Scene");
			}

			if (ImGui::MenuItem("Load"))
			{
				gns::serialization::SceneSerializer serializer;
				serializer.DeserializeScene(PathManager::FromAssetsRelative("scene.gnsscene"));
				LOG_INFO("Load Scene");
			}

			if (ImGui::MenuItem("Save As ...", "(Shift+Ctrl+S)"))
			{
				LOG_INFO("SaveScene As ...");
			}
			if (ImGui::MenuItem("Open...", "(Ctrl+O)"))
			{
				LOG_INFO("Open Scene ... ");
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Import ..."))
			{
				LOG_INFO("Import Asset(s)... ");
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit")) {
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Window")) {
			if(ImGui::MenuItem("Content Browser"))
			{
				*sWindowMenuMap["contentBrowserWindow"] = true;
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

DockSpaceWindow::~DockSpaceWindow() = default;



void DockSpaceWindow::InitWindow()
{
	m_open = true;
	//GuiWindow::InitWindow();
	return;
}

bool DockSpaceWindow::OnWindowBegin()
{
	//GuiWindow::OnWindowBegin();
	return true;
}

void DockSpaceWindow::OnWindowEnd()
{
	//GuiWindow::OnWindowEnd();
	return;
}
