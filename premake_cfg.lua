-- premake_cfg.lua
-- Unified configuration for GenesisEngine Premake

---------------------------------------
-- SDK / SYSTEM PATHS
---------------------------------------

VULKAN_SDK = os.getenv("VULKAN_SDK")

Paths = {
    Vulkan_Include = VULKAN_SDK .. "/Include",
    Vulkan_Lib     = VULKAN_SDK .. "/Lib",
    SDL_Include    = VULKAN_SDK .. "/Include/SDL2",

    ProjectDir     = "D:\\Project_Genesis\\GenesisEngine_TestProject",
    ResourceDir    = "D:\\Project_Genesis\\GenesisEngine\\Resources",

    OutputDir      = "bin/%{cfg.buildcfg}",
    IntermediateDir= "bin-int"
}

---------------------------------------
-- SUBMODULE ROOTS
---------------------------------------
Submodules = {
    Assimp       = path.getabsolute("submodules/assimp"),
    YAMLCPP      = path.getabsolute("submodules/yaml-cpp"),
    ImGui        = path.getabsolute("submodules/imgui"),
    ImGuizmo     = path.getabsolute("submodules/ImGuizmo"),
    VKBootstrap  = path.getabsolute("submodules/vk-bootstrap"),
    EnTT         = path.getabsolute("submodules/entt"),
    SpirvReflect = path.getabsolute("submodules/spirv_reflect"),
}

---------------------------------------
-- SUBMODULE INCLUDE DIRECTORIES
---------------------------------------
IncludeDir = {
    Vulkan       = Paths.Vulkan_Include,
    SDL          = Paths.SDL_Include,

    -- Engine
    Engine_API     = "Engine/API",
    Engine_Include = "Engine/include",

    -- Submodules
    ImGui          = Submodules.ImGui,
    ImGui_Backends = Submodules.ImGui .. "/backends",
    ImGuizmo       = Submodules.ImGuizmo,

    Assimp         = Submodules.Assimp .. "/include",
    Assimp_Build   = Submodules.Assimp .. "/build/include",

    VKBootstrap    = Submodules.VKBootstrap .. "/src",
    EnTT           = Submodules.EnTT .. "/single_include",
    YAML           = Submodules.YAMLCPP .. "/include",
    SpirvReflect   = Submodules.SpirvReflect
}

---------------------------------------
-- LIBRARY DIRECTORIES
---------------------------------------
LibDir = {
    Output   = Paths.OutputDir,
    Vulkan   = Paths.Vulkan_Lib,
    Assimp   = Submodules.Assimp .. "/build/lib/Release"
}

---------------------------------------
-- THIRD-PARTY STATIC LIBS (file names only)
---------------------------------------
Libs = {
    SDL2             = "SDL2.lib",
    SDL2main         = "SDL2main.lib",
    Vulkan           = "vulkan-1.lib",
    Assimp           = "assimp-vc143-mt.lib",
    Assimp_d         = "assimp-vc143-mtd.lib",
    ImGui            = "ImGui.lib",
    SpirvReflect     = "spirv_reflect.lib",
    YAMLCPPd         = "yaml-cppd.lib",
    YAMLCPP          = "yaml-cpp.lib",
    SpirvCrossCore_d = "spirv-cross-cored.lib",
    SpirvCrossCore   = "spirv-cross-core.lib"
}

---------------------------------------
-- STANDARD OUTPUT DIRECTORIES (shortcuts)
---------------------------------------
SolutionRoot = os.getcwd()

-- <SolutionRoot>/bin/<Project>/<Config>
Out = function(projectName)
    return ("%s/bin/%s/%%{cfg.buildcfg}"):format(SolutionRoot, projectName)
end

-- <SolutionRoot>/bin-int/<Project>/<Config>
Obj = function(projectName)
    return ("%s/bin-int/%s/%%{cfg.buildcfg}"):format(SolutionRoot, projectName)
end

function to_win_path(p)
    return p:gsub("/", "\\")
end
