# GenesisEngine
Vulkan "Game Engine" passion project

Requires Vulkan SDK installed!
Using libraries included in the Vulkan SDK, and expecting the environmet variable VULKAN_SDK to be set.

was not intended to share... i took some shortcuts by setting up the project.

How to build:
1. Clone with submodules.
    * Clone or download the [test project], (This is the PROJECT_DIR you edit in the premake file) if there is no project it will just chrash.
3. Open Premake.lua and edit the Paths PROJECT_DIR and RESOURCE_DIR(This is currently "\GenesisEngine\Resources" should be absolute!)
    * Build the Assimp CMake Project.
    * Build the Assimp libraries in Release configuration.
5. Run Premake to generate the project files.
    * There is no fork For Dear ImGui, go to imgui.h Include the "API.h" and define IMGUI_API to GNS_API
7. After Building the project The Assimp output dll (assimp-vc143-mt.dll) should be copied manually to the output directory bin/debug.

[test project]: https://github.com/varigeri92/GenesisEngine_TestProject
