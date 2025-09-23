#include "PathManager.h"
#include "../Engine/Engine.h"
#include "../Engine/Rendering/GuiWindowDrawer.h"
#include "AssetManagement/AssetImporter.h"
#include "EditorScene/EditorCamera.h"
#include "Gui/Windows/AssetImporterWindow.h"
#include "Gui/Windows/ContentBrowser.h"
#include "Gui/Windows/DockSpaceWindow.h"
#include "Gui/Windows/HierarchyView.h"
#include "Gui/Windows/InspectorWindow.h"
#include "Gui/Windows/MenuWindow.h"
#include "Gui/Windows/SceneView.h"


#define PROJECT_NAME  "Genesis Engine - Editor"

void config_style() {
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;

	// Base Colors
	ImVec4 bgColor = ImVec4(0.10f, 0.105f, 0.11f, 1.00f);
	ImVec4 lightBgColor = ImVec4(0.15f, 0.16f, 0.17f, 1.00f);
	ImVec4 panelColor = ImVec4(0.17f, 0.18f, 0.19f, 1.00f);
	ImVec4 panelHoverColor = ImVec4(0.20f, 0.22f, 0.24f, 1.00f);
	ImVec4 panelActiveColor = ImVec4(0.23f, 0.26f, 0.29f, 1.00f);
	ImVec4 textColor = ImVec4(0.86f, 0.87f, 0.88f, 1.00f);
	ImVec4 textDisabledColor = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	ImVec4 borderColor = ImVec4(0.14f, 0.16f, 0.18f, 1.00f);

	// Text
	colors[ImGuiCol_Text] = textColor;
	colors[ImGuiCol_TextDisabled] = textDisabledColor;

	// Windows
	colors[ImGuiCol_WindowBg] = bgColor;
	colors[ImGuiCol_ChildBg] = bgColor;
	colors[ImGuiCol_PopupBg] = bgColor;
	colors[ImGuiCol_Border] = borderColor;
	colors[ImGuiCol_BorderShadow] = borderColor;

	// Headers
	colors[ImGuiCol_Header] = panelColor;
	colors[ImGuiCol_HeaderHovered] = panelHoverColor;
	colors[ImGuiCol_HeaderActive] = panelActiveColor;

	// Buttons
	colors[ImGuiCol_Button] = panelColor;
	colors[ImGuiCol_ButtonHovered] = panelHoverColor;
	colors[ImGuiCol_ButtonActive] = panelActiveColor;

	// Frame BG
	colors[ImGuiCol_FrameBg] = lightBgColor;
	colors[ImGuiCol_FrameBgHovered] = panelHoverColor;
	colors[ImGuiCol_FrameBgActive] = panelActiveColor;

	// Tabs
	colors[ImGuiCol_Tab] = panelColor;
	colors[ImGuiCol_TabHovered] = panelHoverColor;
	colors[ImGuiCol_TabActive] = panelActiveColor;
	colors[ImGuiCol_TabUnfocused] = panelColor;
	colors[ImGuiCol_TabUnfocusedActive] = panelHoverColor;

	// Title
	colors[ImGuiCol_TitleBg] = bgColor;
	colors[ImGuiCol_TitleBgActive] = bgColor;
	colors[ImGuiCol_TitleBgCollapsed] = bgColor;

	// Scrollbar
	colors[ImGuiCol_ScrollbarBg] = bgColor;
	colors[ImGuiCol_ScrollbarGrab] = panelColor;
	colors[ImGuiCol_ScrollbarGrabHovered] = panelHoverColor;
	colors[ImGuiCol_ScrollbarGrabActive] = panelActiveColor;

	// Checkmark
	colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);

	// Slider
	colors[ImGuiCol_SliderGrab] = panelHoverColor;
	colors[ImGuiCol_SliderGrabActive] = panelActiveColor;

	// Resize Grip
	colors[ImGuiCol_ResizeGrip] = panelColor;
	colors[ImGuiCol_ResizeGripHovered] = panelHoverColor;
	colors[ImGuiCol_ResizeGripActive] = panelActiveColor;

	// Separator
	colors[ImGuiCol_Separator] = borderColor;
	colors[ImGuiCol_SeparatorHovered] = panelHoverColor;
	colors[ImGuiCol_SeparatorActive] = panelActiveColor;

	// Plot
	colors[ImGuiCol_PlotLines] = textColor;
	colors[ImGuiCol_PlotLinesHovered] = panelActiveColor;
	colors[ImGuiCol_PlotHistogram] = textColor;
	colors[ImGuiCol_PlotHistogramHovered] = panelActiveColor;

	// Text Selected BG
	colors[ImGuiCol_TextSelectedBg] = panelActiveColor;

	// Modal Window Dim Bg
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.10f, 0.105f, 0.11f, 0.5f);

	// Tables
	colors[ImGuiCol_TableHeaderBg] = panelColor;
	colors[ImGuiCol_TableBorderStrong] = borderColor;
	colors[ImGuiCol_TableBorderLight] = borderColor;
	colors[ImGuiCol_TableRowBg] = bgColor;
	colors[ImGuiCol_TableRowBgAlt] = lightBgColor;

	// Styles
	style.FrameBorderSize = 1.0f;
	style.FrameRounding = 2.0f;
	style.WindowBorderSize = 1.0f;
	style.PopupBorderSize = 1.0f;
	style.ScrollbarSize = 12.0f;
	style.ScrollbarRounding = 2.0f;
	style.GrabMinSize = 7.0f;
	style.GrabRounding = 2.0f;
	style.TabBorderSize = 1.0f;
	style.TabRounding = 2.0f;

	// Reduced Padding and Spacing
	style.WindowPadding = ImVec2(5.0f, 5.0f);
	style.FramePadding = ImVec2(4.0f, 3.0f);
	style.ItemSpacing = ImVec2(6.0f, 4.0f);
	style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);

	/*
	// Font Scaling
	ImGuiIO& io = ImGui::GetIO();
	io.FontGlobalScale = 0.95f;

	io.Fonts->AddFontDefault();
	float baseFontSize = 18.0f;
	float iconFontSize = baseFontSize * 2.0f / 3.0f;

	// merge in icons from Font Awesome
	static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
	ImFontConfig icons_config;
	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;
	icons_config.GlyphMinAdvanceX = iconFontSize;
	io.Fonts->AddFontFromFileTTF(
		(std::string(RESOURCE_DIR) + "/fonts/" + FONT_ICON_FILE_NAME_FA).c_str(), iconFontSize,
		&icons_config, icons_ranges);
	*/
}

void SetProjectArguments(std::string command, std::string value)
{
	if (command == "-p" || command == "-project" || command == "-proj")
	{
		PathManager::ProjectPath = value;
		char lc = value.back();
		if (lc != '\\')
			PathManager::ProjectPath += "\\";

	}

	if (command == "-r" || command == "-resources" || command == "-res")
	{
		PathManager::ResourcesPath = value;
		char lc = value.back();
		if (lc != '\\')
			PathManager::ResourcesPath += "\\";
	}
}


void SetArguments(int argc, char* argv[])
{
	if (argc > 1)
	{
		for (int i = 1; i < argc; i += 2) {
			SetProjectArguments(argv[i], argv[i + 1]);
		}
	}
	else
	{
		LOG_WARNING("No arguments set, falling back to default filepaths!");
		SetProjectArguments("-p", R"(D:\Project_Genesis\TestProjects\DevTest_Project\)");
		SetProjectArguments("-r", R"(D:\Project_Genesis\GenesisEngine\Resources\)");
	}

	PathManager::AssetDatabasePath = PathManager::FromProjectRelative(R"(.AssetDatabase\)");
	PathManager::AssetsPath = PathManager::FromProjectRelative(R"(Assets\)");
}

int main(int argc, char* argv[])
{
	/*
	PathManager::ProjectPath = R"(D:\Project_Genesis\TestProjects\DevTest_Project\)";
	PathManager::AssetDatabasePath = R"(D:\Project_Genesis\TestProjects\DevTest_Project\.AssetDatabase\)";
	PathManager::AssetsPath = R"(D:\Project_Genesis\TestProjects\DevTest_Project\Assets\)";
	PathManager::ResourcesPath = R"(D:\Project_Genesis\GenesisEngine\Resources\)";
	 */

	SetArguments(argc, argv);
	gns::editor::assets::AssetLibrary::ScanAssetLibrary();

	gns::Engine engine;
	engine.InitEngine(PROJECT_NAME, PathManager::AssetsPath, PathManager::ResourcesPath);
	engine.OnEngineStart([&](){
		LOG_INFO("ON EngineStartCallback");
		config_style();
		//register systems:
		gns::SystemsManager::RegisterSystem<gns::editor::scene::EditorCamera>();
		//create GuiWindows:
		gns::GuiWindowDrawer::CreateGUIWindow<DockSpaceWindow>();
		gns::GuiWindowDrawer::CreateGUIWindow<gns::editor::gui::ContentBrowser>();
		gns::GuiWindowDrawer::CreateGUIWindow<gns::editor::gui::InspectorWindow>();
		gns::GuiWindowDrawer::CreateGUIWindow<gns::editor::gui::HierarchyView>();
		gns::GuiWindowDrawer::CreateGUIWindow<gns::editor::gui::SceneView>();

	});
	/*
	gns::RenderSystem* renderSystem = gns::SystemsManager::GetSystem<gns::RenderSystem>();
	gns::assetLibrary::LoadAsset(R"(D:\_ASSETS\glTF-Sample-Models-main\2.0\Sponza\glTF\sponza.glb)");
	const auto view = gns::SystemsManager::GetRegistry()
		.view<gns::entity::Transform, gns::entity::MeshComponent, gns::entity::EntityComponent>();
	 */

	/*
	const std::string v_shader_path = R"(D:\Project_Genesis\GenesisEngine\Resources\Shaders\colored_triangle_mesh.vert)";
	const std::string f_shader_path = R"(D:\Project_Genesis\GenesisEngine\Resources\Shaders\tex_image.frag)";
	rendering::Shader* shader = CreateShader(v_shader_path, f_shader_path);
	rendering::Material* shared_material = CreateMaterial(shader, "shared_material");
	rendering::Texture* albedoTexture = m_renderer->GetDefaultTexture("white");// CreateTexture(R"(D:\Project_Genesis\TestProjects\DevTest_Project\Assets\Textures\Default_albedo.jpg)");
	rendering::Texture* normalTexture = m_renderer->GetDefaultTexture("blue");//CreateTexture(R"(D:\Project_Genesis\TestProjects\DevTest_Project\Assets\Textures\Default_normal.jpg)");
	rendering::Texture* metallicRoughnessTexture = m_renderer->GetDefaultTexture("white");//CreateTexture(R"(D:\Project_Genesis\TestProjects\DevTest_Project\Assets\Textures\Default_metalRoughness.jpg)");
	rendering::Texture* aoTexture = m_renderer->GetDefaultTexture("white");//CreateTexture(R"(D:\Project_Genesis\TestProjects\DevTest_Project\Assets\Textures\Default_AO.jpg)");
	rendering::Texture* emissionTexture = m_renderer->GetDefaultTexture("white");//CreateTexture(R"(D:\Project_Genesis\TestProjects\DevTest_Project\Assets\Textures\Default_emissive.jpg)");
	*/
	/* 
	for (auto [entt, transform, meshComponent, entity] : view.each())
	{
		gns::guid _guid = meshComponent.mesh_ref;
		renderSystem->UploadMesh(gns::Object::Get<gns::rendering::Mesh>(_guid));
	}
	*/
	/*
	 
	constexpr size_t LIGHT_COUNT = 3;
	for (size_t i = 0; i < LIGHT_COUNT; i++)
	{
		gns::Entity pointLightEntity = gns::Entity::CreateEntity("Point Light_" + std::to_string(i));
		auto& pointlight_comp = pointLightEntity.AddComponet<gns::rendering::PointLightComponent>();
		pointlight_comp.intensity = 1.f;
		pointlight_comp.radius = 10.f;
		auto& colorComponent = pointLightEntity.AddComponet<gns::rendering::ColorComponent>();
		colorComponent.color.r = 1;
		colorComponent.color.g = 1;
		colorComponent.color.b = 1;
		colorComponent.color.a = 1;
		pointLightEntity.GetComponent<gns::entity::Transform>().position = { 0,1.25f,0 };
	}
	 */
	engine.Run();
	engine.ShutDown();
}
