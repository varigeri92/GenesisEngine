#pragma once
#include "../../Engine/GUI/GuiWindow.h"
#include "../EditorGuiUtils/SelectionHandler.h"

namespace gns::editor::gui
{
	class HierarchyView : public gns::gui::GuiWindow
	{
	protected:
		void OnWindowOpen() override;
		void OnWindowClosed() override;
		void OnWindowDraw() override;
		void DrawHierarchyRecursive(entityHandle entity_handle);

	public:
		HierarchyView();
		~HierarchyView() override;

	protected:
		void InitWindow() override;
		bool OnWindowBegin() override;
		void OnWindowEnd() override;

		void DrawHierarchyContextMenu();
		void DrawEntityContextMenu(entityHandle handle);

	public:

	};
}
