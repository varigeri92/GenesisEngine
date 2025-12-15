#pragma once
#include "GenesisGui.h"

namespace gns
{
	class RenderSystem;
}

namespace gns::editor
{
	class LightingSettings final : public gns::gui::GuiWindow
	{
	public:
		LightingSettings();
		~LightingSettings() override;

	protected:
		void OnWindowOpen() override;
		void OnWindowClosed() override;
		void OnWindowDraw() override;
		void InitWindow() override;
		bool OnWindowBegin() override;
		void OnWindowEnd() override;

	private:
		RenderSystem* m_renderSystem;
	};
}
