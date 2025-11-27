-- projects/spirv_reflect.lua

project "spirv_reflect"
    kind "StaticLib"
    targetdir(Paths.OutputDir)
    objdir(Obj("spirv_reflect"))
    location "spirv_reflect"

    libdirs { LibDir.Output }
    defines { "BUILD_ENGINE_LIB" }

    includedirs { IncludeDir.SpirvReflect }

    files {
        Submodules.SpirvReflect .. "/spirv_reflect.cpp",
        Submodules.SpirvReflect .. "/spirv_reflect.h"
    }
