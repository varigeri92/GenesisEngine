#include "SelectionHandler.h"


gns::editor::utils::SelectedItem* gns::editor::utils::SelectionHandler::CurrentSelectedItem = nullptr;
gns::Event gns::editor::utils::SelectionHandler::onSelectionChangeEvent = {};

void gns::editor::utils::SelectionHandler::SetSelectedItem(SelectableItemType itemType, const std::string& path,
	guid itemGuid)
{
	if(CurrentSelectedItem != nullptr)
	{
		delete CurrentSelectedItem;
	}

	CurrentSelectedItem = new SelectedItem {
		itemType, path, itemGuid
	};

	onSelectionChangeEvent.Dispatch();
}

void gns::editor::utils::SelectionHandler::SetSelectedItem(Entity& entity)
{
}
