#pragma once
#include "GenesisGui.h"

namespace gns
{
	class RenderSystem;
}

class Screen;

namespace gns::rendering
{
	struct Camera;
}

class ShadowDebuggerWindow : public gns::gui::GuiWindow
{
protected:
	void OnWindowOpen() override;
	void OnWindowClosed() override;
	void OnWindowDraw() override;

public:
	~ShadowDebuggerWindow() override;

protected:
	void InitWindow() override;
	bool OnWindowBegin() override;
	void OnWindowEnd() override;

private:
	gns::RenderSystem* m_renderSystem;
	ImVec2 m_drawRegion;
	ImTextureID m_renderTexture_shadow;
	ImTextureID m_renderTexture_scene;


};
