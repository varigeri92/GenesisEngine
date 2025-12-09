-- yaml-cpp.lua
-- Build yaml-cpp using external CMake

project "yaml-cpp"
    kind "Utility"

    -- Full absolute paths from configuration (solution-root safe)
    local Yaml_SourceDir = Submodules.YAMLCPP              -- e.g. D:/.../submodules/yaml-cpp
    local Yaml_BuildDir  = Submodules.YAMLCPP .. "/build"  -- e.g. D:/.../submodules/yaml-cpp/build

    --
    -- 1. Ensure the build directory exists
    --
    prebuildcommands {
        ('if not exist "%s" mkdir "%s"'):format(to_win_path(Yaml_BuildDir), to_win_path(Yaml_BuildDir))
    }

    --
    -- 2. Configure (generate) CMake project
    --
    prebuildcommands {
        (
            'cmake -S "%s" -B "%s" '
            .. '-DCMAKE_BUILD_TYPE=%%{cfg.buildcfg} '
            .. '-DYAML_BUILD_SHARED_LIBS=OFF '
            .. '-DYAML_CPP_BUILD_TOOLS=OFF'
        ):format(Yaml_SourceDir, Yaml_BuildDir)
    }

    --
    -- 3. Build yaml-cpp using CMake
    --
    postbuildcommands {
        ('cmake --build "%s" --config %%{cfg.buildcfg}'):format(Yaml_BuildDir)
    }

    --
    -- 4. Copy generated libraries into bin/yaml-cpp build output
    --
    local batch = to_win_path(path.getabsolute("copy_files.bat"))
    local buildDir = Yaml_BuildDir
    local outDir   = to_win_path(path.getabsolute(Paths.OutputDir))
    postbuildcommands {
        ('call "%s" "%s" "%s" "%%{cfg.buildcfg}"'):
            format(batch, buildDir, outDir)
    }

    --
    -- 5. VS must know the produced file so dependencies work
    --
    buildoutputs {
        ('%s/%%{cfg.buildcfg}/yaml-cpp*.lib'):format(Yaml_BuildDir)
    }