dofile("scripts/utils.lua")

configure_file("scripts/config.h.in", "GameLib/src/Game/config.h", {
    OpenGL_FPS_VERSION_MAJOR = "0",
    OpenGL_FPS_VERSION_MINOR = "0",
    OpenGL_FPS_VERSION_PATCH = "9"
})

workspace "OpenGL_FPS"
    architecture "x64"
    startproject "Game"

    configurations
    {
        "Debug",
        "Release"
    }

    flags
    {
        "MultiProcessorCompile"
    }

    filter "system:windows"
        buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "GameLib/Build-GameLib.lua"
include "Game/Build-Game.lua"
