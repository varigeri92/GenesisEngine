#include "InspectorWindow.h"
#include <fstream>
#include <spirv_cross/spirv_cross.hpp>
#include "typedefs.h"
#include "../../PathManager.h"
#include "../../../Engine/AssetDatabase/Serializer/AssetSerializer.h"
#include "../../../Engine/Rendering/RenderSystem.h"
#include "../../../Engine/Rendering/Shader.h"
#include "../../../Engine/Rendering/Objects/Material.h"
#include "../../AssetManagement/AssetImporter.h"
#include "../EditorGuiUtils/SelectionHandler.h"


gns::AssetMetadata* assetMetadata;

std::vector<std::string> textureSlots = {
"Albedo",
"Normal",
"MetallicRoughness",
"AmbientOcclusion",
"Emission"};

static ImGuiTableFlags table_flags = ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_SizingFixedFit
| ImGuiTableFlags_NoPadInnerX | ImGuiTableFlags_NoPadOuterX;

std::unordered_map<size_t, std::function<void(const std::string& name, void* componentData, size_t offset)>> FieldDrawTable
{
	{typeid(std::string).hash_code(),[](const std::string& name, void* componentData, size_t offset)
		{
			ImGui::Text(reinterpret_cast<std::string*>(static_cast<char*>(componentData) + offset)->c_str());
		}
	},
	{typeid(glm::vec3).hash_code(),[](const std::string& name, void* componentData, size_t offset)
		{
			ImGui::DragFloat3(("##" + name).c_str(),
			reinterpret_cast<float*>(static_cast<char*>(componentData) + offset), 0.1f);
		}
	},
	{typeid(gns::guid).hash_code(),[](const std::string& name, void* componentData, size_t offset)
		{
			ImGui::Button( std::to_string(*reinterpret_cast<size_t*>(static_cast<char*>(componentData) + offset)).c_str(), 
				{ ImGui::GetContentRegionAvail().x, 0 });
		}
	},
	{typeid(bool).hash_code(),[](const std::string& name, void* componentData, size_t offset)
		{
			ImGui::Checkbox(("##" + name).c_str(),
			reinterpret_cast<bool*>(static_cast<char*>(componentData) + offset));
		}
	},
	{typeid(float).hash_code(),[](const std::string& name, void* componentData, size_t offset)
		{
			ImGui::DragFloat(("##" + name).c_str(),
			reinterpret_cast<float*>(static_cast<char*>(componentData) + offset), 0.1f);
		}
	},
	{typeid(gns::color4).hash_code(),[](const std::string& name, void* componentData, size_t offset)
		{
			ImGui::ColorEdit4(("##" + name).c_str(),
			reinterpret_cast<float*>(static_cast<char*>(componentData) + offset));
		}
	},
	{typeid(gns::color3).hash_code(),[](const std::string& name, void* componentData, size_t offset)
		{
			ImGui::ColorEdit3(("##" + name).c_str(),
			reinterpret_cast<float*>(static_cast<char*>(componentData) + offset));
		}
	},
};

gns::rendering::Material* current_selected_material;
gns::RenderSystem* renderSystem;

std::vector<gns::rendering::Material*> currentSelectedEntity_materials;


std::vector<uint32_t> LoadShader(std::string path)
{
	std::string p = (path);
	std::ifstream file(p, std::ios::ate | std::ios::binary);
	if (!file.is_open())
	{
		LOG_ERROR("Failed to open file: " + p);
		assert(true);
		return {};
	}
	size_t fileSize = (size_t)file.tellg();
	std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
	file.seekg(0);
	file.read((char*)buffer.data(), fileSize);
	file.close();
	return buffer;
}

void ReadShaderAttributes(gns::rendering::Shader* shader)
{
	std::vector<uint32_t> spirv = LoadShader(shader->fragmentShaderPath);
	spirv_cross::Compiler comp(std::move(spirv));
	spirv_cross::ShaderResources res = comp.get_shader_resources();
	LOG_INFO("Shader Data of: " + shader->fragmentShaderPath);
	LOG_INFO("\t Uniforms:");
	for (size_t i = 0; i < res.uniform_buffers.size(); i++)
	{
		LOG_INFO(res.uniform_buffers[i].name);
	}

	LOG_INFO("\t Sampled Images:");
	for (size_t i = 0; i < res.sampled_images.size(); i++)
	{
		LOG_INFO("\t\t" + res.sampled_images[i].name);
	}

	LOG_INFO("\t Storage buffers:");
	for (size_t i = 0; i < res.gl_plain_uniforms.size(); i++)
	{
		LOG_INFO("\t\t" + res.gl_plain_uniforms[i].name);
	}

	/*
	spirv_cross::Compiler comp(std::move(spirv));

	spirv_cross::ShaderResources res = comp.get_shader_resources();
	LOG_INFO("Shader Data of: " + shader->fragmentShaderPath);
	LOG_INFO("\t Uniforms:");
	for (size_t i = 0; i < res.uniform_buffers.size(); i++)
	{
		LOG_INFO(res.uniform_buffers[i].name);
		auto& type = comp.get_type(res.uniform_buffers[i].type_id);
		uint32_t set = comp.get_decoration(res.uniform_buffers[i].id, spv::DecorationDescriptorSet);
		uint32_t binding = comp.get_decoration(res.uniform_buffers[i].id, spv::DecorationBinding);
		auto member_types = type.member_types;
		auto member_names = comp.get_type(res.uniform_buffers[i].type_id).member_name_cache;
		//if (res.uniform_buffers[i].name == "SceneData")
			//continue;


		for (size_t j = 0; j < member_types.size(); j++) {
			auto name = comp.get_member_name(type.self, j);
			auto member_type = comp.get_type(member_types[i]);
			size_t member_size = comp.get_declared_struct_member_size(type, j);
			size_t offset = comp.type_struct_member_offset(type, j);
		}
	}
	*/
}


gns::editor::gui::InspectorWindow::InspectorWindow()
{
	hasSelection = false;
}

gns::editor::gui::InspectorWindow::~InspectorWindow()
{

}

void gns::editor::gui::InspectorWindow::OnWindowOpen()
{
}

void gns::editor::gui::InspectorWindow::OnWindowClosed()
{
}

void gns::editor::gui::InspectorWindow::OnWindowDraw()
{
	ImGui::SeparatorText("Inspector");
	if(!hasSelection)
	{
		ImGui::Text("Nothing is selected");
		return;
	}

	if (utils::SelectionHandler::Get()->type == utils::SelectableItemType::Entity)
		DrawInspectedEntity();
	else
		DrawInspectedAsset();
	
}
void gns::editor::gui::InspectorWindow::InitWindow()
{
	m_title = "Inspector";
	m_open = true;
	m_flags = ImGuiWindowFlags_None;
	m_menuPath = "";
	EventListener onSelectionChangeListener = { [&]
	{
		auto* selectedItem = utils::SelectionHandler::Get();
		if(selectedItem->type == utils::SelectableItemType::Entity)
		{
			currentEntity = Entity{static_cast<entt::entity>(selectedItem->itemGuid)};
			hasSelection = true;
			m_selectedEntityComponents = currentEntity.GetAllComponent();
			entity::MeshComponent* mesh = nullptr;
			if(currentEntity.TryGetComponent<entity::MeshComponent>(mesh))
			{
				auto& meshComp = currentEntity.GetComponent<entity::MeshComponent>();
				currentSelectedEntity_materials = meshComp.materials;
				current_selected_material = currentSelectedEntity_materials[0];
			}
			else
			{
				current_selected_material = nullptr;
			}
		}
		else
		{
			hasSelection = true;
			currentSelectedAssetPath = selectedItem->path;
			is_imported = assets::AssetImporter::IsImported(currentSelectedAssetPath);
			if (is_imported)
			{
				if (!assets::AssetImporter::IsMeta(currentSelectedAssetPath))
				{
					assetMetadata = assets::AssetImporter::GetMetadata(currentSelectedAssetPath);
					LOG_INFO(assetMetadata->assetName);
				}
			}
			
		}
	}};
	GuiWindow::InitWindow();
	utils::SelectionHandler::onSelectionChangeEvent.AddListener(onSelectionChangeListener);

	renderSystem = SystemsManager::GetSystem<RenderSystem>();
}

bool gns::editor::gui::InspectorWindow::OnWindowBegin()
{
	return GuiWindow::OnWindowBegin();
}

void gns::editor::gui::InspectorWindow::OnWindowEnd()
{
	GuiWindow::OnWindowEnd();
}

void gns::editor::gui::InspectorWindow::DrawInspectedEntity()
{
	constexpr float label_ratio = 0.2f;
	const float available_Width = ImGui::GetContentRegionAvail().x;
	const float label_width = available_Width * label_ratio;

	Entity entity = currentEntity;
	for (ComponentData& component : m_selectedEntityComponents)
	{
		DrawComponent(component);
	}
	if (current_selected_material != nullptr)
	{
		for (gns::rendering::Material* currentSelectedEntityMaterial : currentSelectedEntity_materials)
		{
			ImGui::SeparatorText("Material");
			ImGui::Text(currentSelectedEntityMaterial->name.c_str());

			ImGui::BeginGroup();

			if (ImGui::BeginTable("material", 2, table_flags))
			{
				ImGui::TableSetupColumn("##", ImGuiTableColumnFlags_WidthFixed, label_width);
				ImGui::TableSetupColumn("##", ImGuiTableColumnFlags_WidthFixed, available_Width - label_width);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("Albedo:");
				ImGui::TableNextColumn();
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::ColorEdit4("##AlbedoColor", (float*)&currentSelectedEntityMaterial->uniformData.albedoColor.x);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("Metallic:");
				ImGui::TableNextColumn();
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::DragFloat("##metallic", (float*)&currentSelectedEntityMaterial->uniformData.metallic_roughness_AO.x, 0.01f, 0.f, 1.f);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("Roughness:");
				ImGui::TableNextColumn();
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::DragFloat("##roughness", (float*)&currentSelectedEntityMaterial->uniformData.metallic_roughness_AO.y, 0.01f, 0.f, 1.f);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("Ambient Occlusion:");
				ImGui::TableNextColumn();
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::DragFloat("##ao", (float*)&currentSelectedEntityMaterial->uniformData.metallic_roughness_AO.z, 0.01f, 0.f, 1.f);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("Emission intensity:");
				ImGui::TableNextColumn();
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::DragFloat("##emission", (float*)&currentSelectedEntityMaterial->uniformData.metallic_roughness_AO.w, 0.01f, 0.f, 100.f);

				for (size_t i = 0; i < currentSelectedEntityMaterial->textures.size(); i++)
				{
					rendering::Texture* texture = currentSelectedEntityMaterial->textures[i];
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::Text(textureSlots[i].c_str());
					ImGui::TableNextColumn();
					ImGui::PushID(textureSlots[i].c_str());
					ImGui::Button((texture->name).c_str(), { ImGui::GetContentRegionAvail().x, 0 });
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GNS_ASSET"))
						{
							const std::string& payload_string = *static_cast<std::string*>(payload->Data);
							LOG_INFO(payload_string);
							if (assets::AssetImporter::ImportAsset(payload_string, false))
							{
								AssetMetadata* metadata_ptr = assets::AssetImporter::GetMetadata(payload_string);
								if (metadata_ptr->assetType == assetLibrary::AssetType::Texture)
								{
									rendering::Texture* texture = renderSystem->CreateTexture(PathManager::FromAssetsRelative(metadata_ptr->srcPath));
									currentSelectedEntityMaterial->textures[i] = texture;
								}
							}
						}
						ImGui::EndDragDropTarget();
					}
					ImGui::PopID();
				}
			}

			ImGui::EndTable();
			ImGui::EndGroup();


			if (ImGui::Button("RebuildPipeline"))
			{
				renderSystem->ReCreateShader(currentSelectedEntityMaterial->shader->getGuid());
				ReadShaderAttributes(currentSelectedEntityMaterial->shader);
			}

			if (ImGui::Button("Save Material"))
			{
				guid guid = currentSelectedEntityMaterial->getGuid();
				AssetSerializer assetSerializer;
				assetSerializer.SerializeAsset<gns::rendering::Material>(currentSelectedEntityMaterial);
			}
		}
	}
}

struct meshImportSettings
{
	bool isStatic = true;
	bool importMaterials = true;

	bool importSkeleton = true;
	bool importTextures = true;

	bool generatePrefab = true;
};
meshImportSettings meshImportSettings;

void gns::editor::gui::InspectorWindow::DrawInspectedAsset()
{
	constexpr float third_ratio = 0.33333333f;
	constexpr float label_ratio = 0.6f;
	const float available_Width = ImGui::GetContentRegionAvail().x;
	const float label_width = available_Width * label_ratio;
	const float third_width = available_Width * third_ratio;
	ImVec2 button_size = { third_width, 0 };
	ImGui::Button("Tab 1", button_size);
	ImGui::SameLine();
	ImGui::Button("Tab 2", button_size);
	ImGui::SameLine();
	ImGui::Button("Tab 3", button_size);

	ImGui::SeparatorText("Import Settings");

	ImGui::Text(currentSelectedAssetPath.c_str());
	if (is_imported)
	{
		ImGui::Text("Assest already Imported!");
		ImGui::Text(std::to_string(assetMetadata->assetGuid).c_str());
		ImGui::Text(assetMetadata->assetName.c_str());
		ImGui::Text(assetMetadata->srcPath.c_str());
		ImGui::Text(std::to_string(static_cast<int>(assetMetadata->assetType)).c_str());
		ImGui::Separator();
		if (ImGui::BeginTable("ImportSettings Table", 2, table_flags))
		{
			ImGui::TableSetupColumn("##", ImGuiTableColumnFlags_WidthFixed, label_width);
			ImGui::TableSetupColumn("##", ImGuiTableColumnFlags_WidthFixed, available_Width - label_width);

			if(assetMetadata->assetType == assetLibrary::AssetType::Mesh)
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("Is Static:");
				ImGui::TableNextColumn();
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::Checkbox("##isStatic", &meshImportSettings.isStatic);


				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("Import Materials:");
				ImGui::TableNextColumn();
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::Checkbox("##Import Materials", &meshImportSettings.importMaterials);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("Import Textures:");
				ImGui::TableNextColumn();
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::Checkbox("##ImportTextures", &meshImportSettings.importTextures);
			}
			ImGui::EndTable();
		}
	}
	if(is_imported)
	{
		if(ImGui::Button("Reimport", { available_Width, 0 }))
		{
			assets::AssetImporter::ImportAsset(currentSelectedAssetPath, true);
		}
	}
	else
	{
		if (ImGui::Button("Import", { available_Width, 0 }))
		{
			assets::AssetImporter::ImportAsset(currentSelectedAssetPath, true);
		}
	}
	
}

void gns::editor::gui::InspectorWindow::DrawComponent(ComponentData componentData)
{
	if (ImGui::CollapsingHeader(ISerializeableComponent::sComponentData[componentData.typehash].name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::BeginGroup();
		
		float label_ratio = 0.2f;
		float available_Width = ImGui::GetContentRegionAvail().x;
		float label_width = available_Width * label_ratio;


		if (ImGui::BeginTable(ISerializeableComponent::sComponentData[componentData.typehash].name.c_str(), 2, table_flags))
		{
			ImGui::TableSetupColumn("##", ImGuiTableColumnFlags_WidthFixed, label_width);
			ImGui::TableSetupColumn("##", ImGuiTableColumnFlags_WidthFixed, available_Width - label_width);
			for (FieldMetadata& field : ISerializeableComponent::sComponentData[componentData.typehash].fields)
			{
				DrawField(field, componentData.data);
			}
			ImGui::EndTable();
		}
		ImGui::EndGroup();
	}
}

void gns::editor::gui::InspectorWindow::DrawField(FieldMetadata& field, void* componentData)
{
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	ImGui::Text(field.name.c_str());
	ImGui::TableNextColumn();
	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
	if(FieldDrawTable.contains(field.type))
	{
		FieldDrawTable[field.type](field.name, componentData, field.offset);
	}
	else
	{
		LOG_ERROR("THE given Field ID: '" + std::to_string(field.type) + "' has no associated function!");
	}
	ImGui::PopItemWidth();
}
