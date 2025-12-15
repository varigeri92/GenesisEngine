#pragma once
#include "GenesisGui.h"

namespace gns::editor::gui
{
	class MenuWindow : public gns::gui::GuiWindow
	{
	protected:
		void OnWindowOpen() override;
		void OnWindowClosed() override;
		void OnWindowDraw() override;

	public:
		~MenuWindow() override;

	protected:
		void InitWindow() override;
		bool OnWindowBegin() override;
		void OnWindowEnd() override;

	public:
		
	};
}
