#include "SceneView.h"
#include "Genesis.h"
#include "GenesisRendering.h"
#include "GenesisSystems.h"

#include "../../EditorScene/EditorCamera.h"
#include "../EditorGuiUtils/SelectionHandler.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

#include "../../AssetManagement/AssetImporter.h"

void decomposeMatrix(const glm::mat4& matrix, glm::vec3& scale, glm::quat& rotation, glm::vec3& translation) {
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(matrix, scale, rotation, translation, skew, perspective);
}

gns::editor::gui::SceneView::SceneView()
{
	m_flags = ImGuiWindowFlags_NoMove;
	m_renderSystem = SystemsManager::GetSystem<RenderSystem>();
	m_renderTexture = reinterpret_cast<ImTextureID>(m_renderSystem->GetRenderTargetTexture()->vulkanImage->texture_descriptorSet);
	m_screen = m_renderSystem->GetTargetScren();
	m_lastFrameRender_Width = 0;
	m_lastFrameRender_Height = 0;

	m_camera = &SystemsManager::GetSystem<scene::EditorCamera>()->m_camera;
}

gns::editor::gui::SceneView::~SceneView()
{
}

void gns::editor::gui::SceneView::OnWindowOpen()
{
}

void gns::editor::gui::SceneView::OnWindowClosed()
{
}

void gns::editor::gui::SceneView::OnWindowDraw()
{
	ImGui::Button("T");
	ImGui::SameLine();
	ImGui::Button("R");
	ImGui::SameLine();
	ImGui::Button("S");
	ImGui::SameLine();
	ImGui::Button("B");

	m_drawRegion = ImGui::GetContentRegionAvail();
	ImGuiViewport* vp = ImGui::GetWindowViewport();



	if(static_cast<uint32_t>(m_drawRegion.x) != m_lastFrameRender_Width ||
		static_cast<uint32_t>(m_drawRegion.y) != m_lastFrameRender_Height)
	{

		m_lastFrameRender_Width = static_cast<uint32_t>(m_drawRegion.x);
		m_lastFrameRender_Height  = static_cast<uint32_t>(m_drawRegion.y);

		m_screen->width = m_lastFrameRender_Width;
		m_screen->height = m_lastFrameRender_Height;
		m_renderTexture = reinterpret_cast<ImTextureID>(m_renderSystem->GetRenderTargetTexture()->vulkanImage->texture_descriptorSet);
		m_screen->aspectRatio = m_drawRegion.x / m_drawRegion.y;
	}


	if(m_screen->updateRenderTargetTarget)
	{
		m_renderTexture = reinterpret_cast<ImTextureID>(m_renderSystem->GetRenderTargetTexture()->vulkanImage->texture_descriptorSet);
		m_screen->updateRenderTargetTarget = false;
	}
	ImGui::Image(m_renderTexture, m_drawRegion);
	if(ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GNS_ASSET"))
		{
			const std::string& payload_string = *static_cast<std::string*>(payload->Data);
			LOG_INFO(payload_string);
			if(assets::AssetImporter::ImportAsset(payload_string, false))
			{
				AssetMetadata* metadata_ptr = assets::AssetImporter::GetMetadata(payload_string);
				if (metadata_ptr->assetType == assetLibrary::AssetType::Mesh)
				{
					const MeshAsset meshAsset = assets::AssetImporter::GetMeshAsset(*metadata_ptr);
					assetLibrary::LoadMeshAsset(meshAsset, 
						[metadata_ptr](const std::vector<guid>& loadedMeshes, const std::vector<guid>& loadedMaterials)
					{
						Entity entity = Entity::CreateEntity(metadata_ptr->assetName);
						entity::MeshComponent& mesh_cmp = entity.AddComponet<entity::MeshComponent>();
						for (size_t i  =0; i < loadedMeshes.size(); i++)
						{
							mesh_cmp.meshes.push_back(Object::Get<rendering::Mesh>(loadedMeshes[i]));
							mesh_cmp.materials.push_back(Object::Get<rendering::Material>(loadedMaterials[i]));
						}
					});
				}
			}
		}
		ImGui::EndDragDropTarget();
	}
	if(utils::SelectionHandler::Get() != nullptr && utils::SelectionHandler::Get()->type == utils::SelectableItemType::Entity)
	{
		Entity entity((entt::entity)utils::SelectionHandler::Get()->itemGuid);
		entity::Transform& transform = entity.GetComponent<entity::Transform>();
		ImGui::SameLine();
		ImGui::Text(entity.GetComponent<entity::EntityComponent>().name.c_str());



		glm::mat4 gridMatrix = glm::mat4(1);
		glm::mat4 t_matrix = transform.matrix;
		glm::mat4 view = m_camera->m_view;
		glm::mat4 projection = m_camera->m_projection;
		projection[1][1] *= -1;
		glm::mat4 i_view = glm::inverse(view);


		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();

		ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + ImGui::GetTextLineHeightWithSpacing()*2,
			(float)m_drawRegion.x, (float)m_drawRegion.y);

		ImGuizmo::Manipulate((float*)&view, (float*)&projection,
			ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::MODE::WORLD, (float*)&t_matrix, NULL, NULL);

		if (ImGuizmo::IsUsing())
		{
			glm::quat rotation = {};
			decomposeMatrix(t_matrix, transform.scale, rotation, transform.position);
			transform.rotation = glm::eulerAngles(rotation);
		}
	}
}

void gns::editor::gui::SceneView::InitWindow()
{
	m_title = "SceneView";
	m_open = true;
	m_flags = ImGuiWindowFlags_None;
	m_menuPath = "";
	GuiWindow::InitWindow();
}

bool gns::editor::gui::SceneView::OnWindowBegin()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0,0 });
	return GuiWindow::OnWindowBegin();

}

void gns::editor::gui::SceneView::OnWindowEnd()
{
	GuiWindow::OnWindowEnd();
	ImGui::PopStyleVar();
}
