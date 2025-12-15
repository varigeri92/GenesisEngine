#pragma once
#include "GenesisGui.h"

namespace gns
{
	struct AssetMetadata;
}

namespace gns::rendering
{
	struct Camera;
}

class Screen;

namespace gns
{
	class RenderSystem;
}

namespace gns::editor::gui
{
	class SceneView : public gns::gui::GuiWindow
	{
	private:
		RenderSystem* m_renderSystem;
		uint32_t m_lastFrameRender_Width;
		uint32_t m_lastFrameRender_Height;
		Screen* m_screen;
		ImVec2 m_drawRegion;
		ImTextureID m_renderTexture;
		rendering::Camera* m_camera;

		void CreateMesh(AssetMetadata* metaData);
	public:
		SceneView();
		~SceneView() override;
	protected:
		void OnWindowOpen() override;
		void OnWindowClosed() override;
		void OnWindowDraw() override;

		void InitWindow() override;
		bool OnWindowBegin() override;
		void OnWindowEnd() override;
	};
}
