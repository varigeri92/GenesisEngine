-- projects/Engine.lua

project "Engine"
    kind "SharedLib"
    targetdir(Paths.OutputDir)
    objdir(Obj("Engine"))
    location "Engine"

    pchheader "gnspch.h"
    pchsource "Engine/gnspch.cpp"

    dependson { "ImGui", "yaml-cpp", "assimp" }

    libdirs {
        LibDir.Assimp,
        LibDir.Output,
        LibDir.Vulkan
    }

    links {
        Libs.SDL2,
        Libs.SDL2main,
        Libs.Vulkan,
        Libs.Assimp,
        Libs.ImGui,
    }

    filter { "configurations:not Debug" }
        links { Libs.YAMLCPP }        -- Release library

    -- Debug-only library
    filter { "configurations:Debug" }
        links { Libs.YAMLCPPd }

    filter {}

    defines { "BUILD_ENGINE_LIB", "GLM_ENABLE_EXPERIMENTAL" }

    includedirs {
        IncludeDir.Vulkan,
        IncludeDir.SDL,
        IncludeDir.Engine_API,
        IncludeDir.Engine_Include,
        IncludeDir.ImGui,
        IncludeDir.Assimp,
        IncludeDir.Assimp_Build,
        IncludeDir.VKBootstrap,
        IncludeDir.EnTT,
        IncludeDir.YAML,
        IncludeDir.ImGuizmo
    }

    files { 
        "Engine/**.h",
        "Engine/**.c",
        "Engine/**.cpp",
        "Engine/**.hpp",

        -- vk-bootstrap
        "submodules/vk-bootstrap/src/*.cpp",
        "submodules/vk-bootstrap/src/*.h"
    }

    filter { 'files:imgui/**.cpp' }
        flags { 'NoPCH' }

    filter { 'files:submodules/vk-bootstrap/src/*.cpp' }
        flags { 'NoPCH' }