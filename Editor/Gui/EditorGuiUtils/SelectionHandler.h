#pragma once
#include <string>
#include "../Engine.h"
#include "../../../Engine/EventSystem/Event.h"

namespace gns::editor::utils
{
	enum class SelectableItemType
	{
		Asset, Entity
	};
	struct SelectedItem
	{
		SelectableItemType type;
		std::string path;
		guid itemGuid;
	};

	class SelectionHandler
	{
		static SelectedItem* CurrentSelectedItem;
	public:

		static Event onSelectionChangeEvent;
		static void SetSelectedItem(SelectableItemType itemType, const std::string& path, guid itemGuid);

		static void SetSelectedItem(Entity& entity);
		static SelectedItem* Get() { return CurrentSelectedItem; }

		static void SetSelected();
		static void OnSelectionChange();
	};
}
