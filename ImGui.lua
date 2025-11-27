-- projects/ImGui.lua

project "ImGui"
    kind "StaticLib"
    targetdir(LibDir.Output)
    objdir(Obj("ImGui"))
    location "ImGui"

    libdirs { LibDir.Output }

    defines { "BUILD_ENGINE_LIB" }

    includedirs {
        "vendor/include",
        IncludeDir.Vulkan,
        IncludeDir.SDL,
        IncludeDir.Engine_API,
        IncludeDir.Engine_Include,
        IncludeDir.ImGui,
        IncludeDir.Assimp,
        IncludeDir.Assimp_Build,
        IncludeDir.VKBootstrap,
        IncludeDir.EnTT
    }

    files {
        -- ImGui core + backends
        IncludeDir.ImGui_Backends .. "/imgui_impl_vulkan.h",
        IncludeDir.ImGui_Backends .. "/imgui_impl_vulkan.cpp",
        IncludeDir.ImGui_Backends .. "/imgui_impl_sdl2.h",
        IncludeDir.ImGui_Backends .. "/imgui_impl_sdl2.cpp",

        Submodules.ImGui .. "/*.h",
        Submodules.ImGui .. "/*.cpp",

        -- ImGuizmo
        Submodules.ImGuizmo .. "/ImGuizmo.cpp",
        Submodules.ImGuizmo .. "/ImGuizmo.h"
    }
