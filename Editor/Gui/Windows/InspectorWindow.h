#pragma once
#include "../../Engine/GUI/GuiWindow.h"
#include "GenesisSystems.h"

namespace gns::editor::gui
{
	class InspectorWindow : public gns::gui::GuiWindow
	{
	public:
		InspectorWindow();
		~InspectorWindow() override;

	protected:
		void OnWindowOpen() override;
		void OnWindowClosed() override;
		void OnWindowDraw() override;

		void InitWindow() override;
		bool OnWindowBegin() override;
		void OnWindowEnd() override;

	private:
		Entity currentEntity;
		std::string currentSelectedAssetPath;
		guid currentSelectedAssetGuid;
		bool hasSelection;
		bool is_imported;
		
		std::vector<ComponentData> m_selectedEntityComponents;

		void DrawInspectedEntity();
		void DrawInspectedAsset();
		void DrawComponent(ComponentData componentData);
		void DrawField(FieldMetadata& field, void* componentData);

	};
}
