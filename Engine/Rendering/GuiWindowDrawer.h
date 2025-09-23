#pragma once

namespace gns::gui
{
	class GuiWindow;
}

namespace gns
{
	class GuiWindowDrawer
	{
		GNS_API static std::vector<gns::gui::GuiWindow*> GuiWindows;
	public:

		static void DrawWindows();

		template<typename T, typename... Args>
		static std::enable_if_t<std::is_base_of_v<gns::gui::GuiWindow, T>, T*> CreateGUIWindow(Args&& ... args)
		{
			T* newWindow = new T{ std::forward<Args>(args)... };
			static_cast<gui::GuiWindow*>(newWindow)->InitWindow();
			GuiWindows.push_back(newWindow);
			return newWindow;
		}
	};
}
