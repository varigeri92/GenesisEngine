#include "HierarchyView.h"
#include "Genesis.h"
#include "GenesisSystems.h"
#include "../../../Engine/ECS/EntitySerializer/EntitySerializer.h"
#include "../../../Engine/Scene/SceneManager.h"
#include "../../EditorEntityHelper/EditorEntityManager.h"

#include "../EditorGuiUtils/SelectionHandler.h"


gns::editor::gui::HierarchyView::HierarchyView() = default;
gns::editor::gui::HierarchyView::~HierarchyView() = default;
void gns::editor::gui::HierarchyView::OnWindowOpen()
{
}

void gns::editor::gui::HierarchyView::OnWindowClosed()
{
}

void gns::editor::gui::HierarchyView::OnWindowDraw()
{
	DrawHierarchyContextMenu();

	for (const scene::Scene& loadedScene : scene::SceneManager::LoadedScenes)
	{
		DrawHierarchyRecursive(loadedScene.sceneRootEntity);
		/*
		ImGui::SeparatorText(loadedScene.m_name.c_str());
		for (const entityHandle & entity_handle : loadedScene.m_entities)
		{
			Entity entity(entity_handle);
			std::string name = entity.GetComponent<entity::EntityComponent>().name;

			ImGui::PushID(ImGui::GetID(static_cast<uint32_t>(entity_handle)));
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanAvailWidth;
			if(utils::SelectionHandler::Get() != nullptr)
			{
				if (utils::SelectionHandler::Get()->itemGuid == static_cast<uint32_t>(entity_handle))
					flags |= ImGuiTreeNodeFlags_Selected;
			}

			bool node_open = ImGui::TreeNodeEx(name.c_str(), flags);
			if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
			{
				utils::SelectionHandler::SetSelectedItem(utils::SelectableItemType::Entity, "", static_cast<size_t>(entity_handle));
				LOG_INFO("Selecting Entity: " + name);
			}
			if (node_open)
			{
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		*/
	}
}

void gns::editor::gui::HierarchyView::DrawHierarchyRecursive(entityHandle entity_handle)
{
	gns::Entity entity = gns::Entity(entity_handle);

	ImGui::PushID(ImGui::GetID(static_cast<uint32_t>(entity_handle)));
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth;
	if (utils::SelectionHandler::Get() != nullptr)
	{
		if (utils::SelectionHandler::Get()->type == utils::SelectableItemType::Entity 
			&& utils::SelectionHandler::Get()->itemGuid == static_cast<uint32_t>(entity_handle))
			flags |= ImGuiTreeNodeFlags_Selected;
	}
	if (entity.Children().size() == 0)
		flags |= ImGuiTreeNodeFlags_Leaf;


	bool node_open = ImGui::TreeNodeEx(entity.Name().c_str(), flags);
	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
	{
		 utils::SelectionHandler::SetSelectedItem(utils::SelectableItemType::Entity, "", static_cast<size_t>(entity_handle));
	}
	DrawEntityContextMenu(entity.entity_handle);
	if (node_open)
	{
		for (const gns::entityHandle& child_handle : entity.Children())
		{
			if (child_handle == entity_handle)
			{
				LOG_ERROR("The entity is the Child Of itself...");
				continue;
			}
			DrawHierarchyRecursive(child_handle);
		}
		ImGui::TreePop();
	}
	ImGui::PopID();
}


void gns::editor::gui::HierarchyView::InitWindow()
{
	m_title = "Hierarchy";
	m_open = true;
	m_flags = 0;
	m_menuPath = "";

	EventListener listener = { [&]
	{
		LOG_INFO("Hello Selection");
	} };
	GuiWindow::InitWindow();

	utils::SelectionHandler::onSelectionChangeEvent.AddListener(listener);
}

bool gns::editor::gui::HierarchyView::OnWindowBegin()
{
	m_open = true;
	return GuiWindow::OnWindowBegin();
}

void gns::editor::gui::HierarchyView::OnWindowEnd()
{
	GuiWindow::OnWindowEnd();
}

void gns::editor::gui::HierarchyView::DrawHierarchyContextMenu()
{
	if (ImGui::IsMouseReleased(1) && ImGui::IsWindowHovered())
		ImGui::OpenPopup("hierarchy_popup");

	if (ImGui::BeginPopup("hierarchy_popup"))
	{
		ImGui::Separator();
		if (ImGui::BeginMenu("Create"))
		{
			if (ImGui::MenuItem("Empty Entity"))
			{
				entity::EditorEntityManager manager;
				manager.CreateNewEntity(entity::PreconfiguredEntityType::Empty);
			}
			if (ImGui::BeginMenu("Lighting"))
			{
				if (ImGui::MenuItem("Sky Light"))
				{
					LOG_INFO("Create New Entity");
				}

				if (ImGui::MenuItem("Directional Light"))
				{
					entity::EditorEntityManager manager;
					manager.CreateNewEntity(entity::PreconfiguredEntityType::DirectionalLight);
				}

				if (ImGui::MenuItem("Point Light"))
				{
					entity::EditorEntityManager manager;
					manager.CreateNewEntity(entity::PreconfiguredEntityType::PointLight);
				}

				if (ImGui::MenuItem("Spot Light"))
				{
					entity::EditorEntityManager manager;
					manager.CreateNewEntity(entity::PreconfiguredEntityType::SpotLight);
				}

				if (ImGui::BeginMenu("Advanced Lighting"))
				{
					if (ImGui::MenuItem("Probes"))
					{
						LOG_INFO("Create New Entity");
					}

					if (ImGui::MenuItem("Cube Map"))
					{
						LOG_INFO("Create New Entity");
					}

					ImGui::EndMenu();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Advanced Lighting"))
			{
				if (ImGui::MenuItem("Probes"))
				{
					LOG_INFO("Create New Entity");
				}

				if (ImGui::MenuItem("Cube Map"))
				{
					LOG_INFO("Create New Entity");
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}

		ImGui::Separator();
		ImGui::EndPopup();
	}
}

void gns::editor::gui::HierarchyView::DrawEntityContextMenu(entityHandle handle)
{
	if (ImGui::IsMouseReleased(1) && ImGui::IsItemHovered())
		ImGui::OpenPopup("entity_popup");

	if (ImGui::BeginPopup("entity_popup"))
	{

		if (ImGui::MenuItem("Delete entity"))
		{
			LOG_INFO("delete entity: " + std::to_string((size_t)handle));
		}

		if (ImGui::MenuItem("Delete with children"))
		{
			LOG_INFO("delete entity with children: " + std::to_string((size_t)handle));
		}
		ImGui::Separator();
		ImGui::EndPopup();
	}
}
