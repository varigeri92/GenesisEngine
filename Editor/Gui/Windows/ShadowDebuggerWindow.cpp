#include "ShadowDebuggerWindow.h"
#include "Genesis.h"
#include "GenesisRendering.h"
#include "GenesisSystems.h"
#include "../EditorGuiUtils/SelectionHandler.h"
void ShadowDebuggerWindow::OnWindowOpen()
{
}

void ShadowDebuggerWindow::OnWindowClosed()
{
}

bool draw = false;
bool togleView = false;
void ShadowDebuggerWindow::OnWindowDraw()
{
	m_drawRegion = ImGui::GetContentRegionAvail();
	ImGui::Image(m_renderTexture_shadow, m_drawRegion);
}

ShadowDebuggerWindow::~ShadowDebuggerWindow() = default;

void ShadowDebuggerWindow::InitWindow()
{
	m_title = "Shadow Debugger";
	m_open = true;
	m_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse;
	gns::rendering::Texture* t = gns::Object::Find<gns::rendering::Texture>("shadow_map_debug");
	m_renderSystem = gns::SystemsManager::GetSystem<gns::RenderSystem>();
	m_renderTexture_shadow = m_renderSystem->GetImGuiTexture(t->handle);
	m_renderTexture_scene = m_renderSystem->GetRenderTargetTextureID();

	GuiWindow::InitWindow();
}

bool ShadowDebuggerWindow::OnWindowBegin()
{
	return GuiWindow::OnWindowBegin();
}

void ShadowDebuggerWindow::OnWindowEnd()
{
	GuiWindow::OnWindowEnd();
}
