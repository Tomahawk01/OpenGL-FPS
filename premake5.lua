dofile("scripts/utils.lua")

configure_file("scripts/config.h.in", "GameLib/src/config.h", {
    OpenGL_FPS_VERSION_MAJOR = "0",
    OpenGL_FPS_VERSION_MINOR = "0",
    OpenGL_FPS_VERSION_PATCH = "21"
})

workspace "OpenGL_FPS"
    architecture "x64"
    startproject "Game"
    toolset "clang"

    configurations
    {
        "Debug",
        "Release"
    }

    flags
    {
        "MultiProcessorCompile"
    }

    filter { "system:windows", "toolset:clang" }
        buildoptions { "/std:c++latest", "-Wno-c23-extensions" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "GameLib/Build-GameLib.lua"
include "Game/Build-Game.lua"
