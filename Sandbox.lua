-- projects/Sandbox.lua

project "Sandbox"
    kind "ConsoleApp"
    targetdir(Paths.OutputDir)
    objdir(Obj("Sandbox"))
    location "Sandbox"

    libdirs {
        LibDir.Output
    }

    links {
        "Engine.lib",
        Libs.ImGui
    }

    defines { "BUILD_SANDBOX_EXE", "GLM_ENABLE_EXPERIMENTAL" }

    includedirs {
        IncludeDir.Vulkan,
        IncludeDir.SDL,
        IncludeDir.Engine_API,
        IncludeDir.Engine_Include,
        "imgui",
        IncludeDir.Assimp,
        IncludeDir.Assimp_Build,
        IncludeDir.VKBootstrap,
        IncludeDir.EnTT
    }

    files {
        "Sandbox/**.h",
        "Sandbox/**.c",
        "Sandbox/**.cpp",
        "Sandbox/**.hpp"
    }

    dependson { "Engine", "ImGui" }

    debugargs { "-p", Paths.ProjectDir, "-r", Paths.ResourceDir, "-c", "??" }