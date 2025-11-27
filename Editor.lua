-- projects/Editor.lua

project "Editor"
    kind "ConsoleApp"
    targetdir(Paths.OutputDir)
    objdir(Obj("Editor"))
    location "Editor"

    libdirs { 
        LibDir.Output,
        LibDir.Vulkan,
        LibDir.Assimp
    }

    links {
        "Engine.lib",
        Libs.SpirvReflect,
        Libs.Assimp,
    }
    
    filter { "configurations:not Debug" }
        links { Libs.YAMLCPP, Libs.SpirvCrossCore }

    filter { "configurations:Debug" }
        links { Libs.YAMLCPPd, Libs.SpirvCrossCore_d }

    filter {}

    defines { "BUILD_SANDBOX_EXE", "GLM_ENABLE_EXPERIMENTAL" }

    includedirs {
        "vendor/include",
        IncludeDir.Vulkan,
        IncludeDir.SDL,
        IncludeDir.Engine_API,
        IncludeDir.Engine_Include,
        IncludeDir.Assimp,
        IncludeDir.Assimp_Build,
        IncludeDir.VKBootstrap,
        IncludeDir.EnTT,
        IncludeDir.ImGui,
        IncludeDir.ImGuizmo,
        IncludeDir.YAML,
        IncludeDir.SpirvReflect
    }

    files {
        "Editor/**.h",
        "Editor/**.c",
        "Editor/**.cpp",
        "Editor/**.hpp"
    }

    debugargs { "-p", Paths.ProjectDir, "-r", Paths.ResourceDir }

    dependson { "Engine", "ImGui", "spirv_reflect", "yaml-cpp", "assimp" }
