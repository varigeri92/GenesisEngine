outDir = "bin/%{cfg.buildcfg}"
VULKAN_SDK = os.getenv("VULKAN_SDK")
VulkanLibDir = VULKAN_SDK .. "/Lib"
VulkanIncludeDir =  VULKAN_SDK .. "/Include"
SDL_DIR = VULKAN_SDK .. "/Include/SDL2"
PROJECT_DIR = "D:\\Project_Genesis\\GenesisEngine_TestProject"
RESOURCE_DIR = "D:\\Project_Genesis\\GenesisEngine\\Resources"

--cmake submodules/assimp/CMakeLists.txt -B submodules/assimp/build

workspace "GenesisEngine"
    language "C++"
    cppdialect "C++20"
    configurations { "Debug", "Release", "Profile", "Dist" }
    platforms { "Win64" }
    startproject "Editor"
    toolset "msc"
    debugenvs { "TRACEDESIGNTIME = true" }
    filter { "platforms:Win64" }
        system "Windows"
        architecture "x86_64"

    filter "configurations:Debug"
        defines { "DEBUG", "_DEBUG", "_CONSOLE" }
        symbols "On"

    filter "configurations:Profile"
        defines { "DEBUG", "_DEBUG", "_CONSOLE", "ENABLE_PROFILER", "TRACE_ALLOCATION" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"

    filter "configurations:Dist"
        defines { "NDEBUG" }
        optimize "On"

project "Engine"
    kind "SharedLib"
    targetdir(outDir)
    objdir "bin-int/Engine/%{cfg.buildcfg}"
    location "Engine" 
    pchheader "gnspch.h"
    pchsource "Engine/gnspch.cpp"

    libdirs { 
        "submodules/assimp/build/lib/Release",
        outDir,
        VulkanLibDir
    }
    links {
        "SDL2.lib",
        "SDL2main.lib",
        "vulkan-1.lib",
        "assimp-vc143-mt.lib",
        "ImGui.lib",
        "yaml-cppd.lib"
    }
    defines {"BUILD_ENGINE_LIB", "GLM_ENABLE_EXPERIMENTAL"}
    includedirs { 
        VulkanIncludeDir,
        SDL_DIR,
        "Engine/API",
        "Engine/include",
        "submodules/imgui",
        "submodules/assimp/include",
        "submodules/assimp/build/include",
        "submodules/vk-bootstrap/src",
        "submodules/entt/single_include",
        "submodules/yaml-cpp/include",
        "submodules/ImGuizmo" }
    files { 
        "Engine/**.h", 
        "Engine/**.c", 
        "Engine/**.cpp", 
        "Engine/**.hpp",
        --vkbootstrap
        "submodules/vk-bootstrap/src/*.cpp",
        "submodules/vk-bootstrap/src/*.h",
        }
    filter { 'files:imgui/**.cpp' }
        flags { 'NoPCH' }
    filter { 'files:submodules/vk-bootstrap/src/*.cpp' }
        flags { 'NoPCH' }
        
project "Sandbox"
    kind "ConsoleApp"
    targetdir(outDir)
    objdir "bin-int/Sandbox/%{cfg.buildcfg}"
    location "Sandbox"
    libdirs { outDir }
    links { "Engine.lib", "ImGui.lib"}
    defines {"BUILD_SANDBOX_EXE", "GLM_ENABLE_EXPERIMENTAL"}
    includedirs {
        VulkanIncludeDir,
        SDL_DIR,
        "Engine/API",
        "Engine/include",
        "imgui",
        "submodules/assimp/include",
        "submodules/assimp/build/include",
        "submodules/vk-bootstrap/src",
        "submodules/entt/single_include" 
    }
    files { "Sandbox/**.h", "Sandbox/**.c", "Sandbox/**.cpp", "Sandbox/**.hpp" }
    dependson { "Engine", "Game", "ImGui"}
    debugargs { "-p", PROJECT_DIR, "-r", RESOURCE_DIR, "-c", "??"}

project "ImGui"
    kind "StaticLib"
    targetdir(outDir)
    objdir "bin-int/ImGui/%{cfg.buildcfg}"
    location "ImGui"
    libdirs { outDir }
    defines {"BUILD_ENGINE_LIB"}
    includedirs {
        "vendor/include",
        VulkanIncludeDir,
        SDL_DIR,
        "Engine/API",
        "Engine/include",
        "submodules/imgui",
        "submodules/assimp/include",
        "submodules/assimp/build/include",
        "submodules/vk-bootstrap/src",
        "submodules/entt/single_include" }
    files { 
        --imgui: 
        "submodules/imgui/backends/imgui_impl_vulkan.h",
        "submodules/imgui/backends/imgui_impl_vulkan.cpp",
        "submodules/imgui/backends/imgui_impl_sdl2.h",
        "submodules/imgui/backends/imgui_impl_sdl2.cpp",
        "submodules/imgui/*.h",
        "submodules/imgui/*.cpp",
        --imguizmo
        "submodules/ImGuizmo/ImGuizmo.cpp",
        "submodules/ImGuizmo/ImGuizmo.h"
        }

project "spirv_reflect"
        kind "StaticLib"
        targetdir(outDir)
        objdir "bin-int/spriv_reflect/%{cfg.buildcfg}"
        location "spirv_reflect"
        libdirs { outDir }
        defines {"BUILD_ENGINE_LIB"}
        includedirs {}
        files { 
            --imgui: 
            "submodules/spirv_reflect/spirv_reflect.cpp",
            "submodules/spirv_reflect/spirv_reflect.h"
            }
    
project "Editor"
    kind "ConsoleApp"
    targetdir(outDir)
    objdir "bin-int/Editor/%{cfg.buildcfg}"
    location "Editor"
    libdirs { outDir, VulkanLibDir, "submodules/assimp/build/lib/Release" }
    links {
        "Engine.lib",
        "spirv_reflect.lib",
        "yaml-cppd.lib",
        "assimp-vc143-mt.lib",
        "spirv-cross-cored.lib"
    }
    defines {"BUILD_SANDBOX_EXE", "GLM_ENABLE_EXPERIMENTAL"}
    includedirs {
        "vendor/include",
        VulkanIncludeDir,
        SDL_DIR,
        "Engine/API",
        "Engine/include",
        "submodules/assimp/include",
        "submodules/assimp/build/include",
        "submodules/vk-bootstrap/src",
        "submodules/entt/single_include",
        "submodules/imgui",
        "submodules/ImGuizmo",
        "submodules/yaml-cpp/include",
        "submodules/spirv_reflect" }
    files { 
        "Editor/**.h", 
        "Editor/**.c", 
        "Editor/**.cpp", 
        "Editor/**.hpp",
    }
    debugargs { "-p", PROJECT_DIR, "-r", RESOURCE_DIR }
    dependson { "Engine", "Game", "ImGui", "spirv_reflect"}