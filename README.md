# GenesisEngine
Vulkan "Game Engine" passion project
* Requires [Vulkan SDK] installed!
* Project Files are generated with [Premake5], submodules require [CMake] for build.
* Using Includes and Libraries From the Vulkan SDK install (glm, spirv, vma, etc)

# How to build:
1. Clone with submodules.
    * Clone or download the [Test Project]. This is required, as the Editor Expecting a valid Project, and there is no project creation.
2. Edit premake_cfg.lua. "ProjectDir" should be the path to the test project you downloaded,
3. Run "setup.bat"  Visual Studio 2022 project files are generated
4. In VisualStudio make sure, that "Editor" is set as the Startup project. 

#Or alternatively Download the build from the relesease

![alt text](https://github.com/varigeri92/GenesisEngine/blob/main/git_images/Screenshot%202025-09-24%20132411.png "screenshot")

* To load the Test Scene Click on File / Load 
* To Save you changes click on File / Save.

There is no file dialog. it will load "X:\Project\Path\Assets\scene.gnsscene" and on save it will save it over!

Scene Loading are currently additive only! Already existing scenenes will not get unloaded.
Hold Right MB to look around and WASD to fly around.

The "Release" build config has some Linking issues. Use "Debug".

[Test Project]: https://github.com/varigeri92/GenesisEngine_TestProject
[Vulkan SDK]: https://vulkan.lunarg.com/sdk/home#windows
[Premake5]: https://premake.github.io
[CMake]: https://cmake.org
