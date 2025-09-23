#pragma once
#include "API.h"
#include "imgui.h"
#include "../Object/Guid.h"

namespace gns
{
	class GuiWindowDrawer;
}

namespace gns::gui
{
	
	class GuiWindow
	{
		friend class GuiWindowDrawer;
	public:
		virtual ~GuiWindow() = default;
		std::string m_title;
		std::string m_menuPath; 
		bool m_open;
		ImGuiWindowFlags m_flags  = 0;
	protected:
		virtual void OnWindowOpen() = 0;
		virtual void OnWindowClosed() = 0;
		GNS_API virtual void InitWindow();
		GNS_API virtual bool OnWindowBegin();
		virtual void OnWindowDraw() = 0;
		GNS_API virtual void OnWindowEnd();

	};
}
