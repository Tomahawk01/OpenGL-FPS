project "GameLib"
    kind "StaticLib"
    language "C++"
    cppdialect "C++23"
    staticruntime "off"

    files
    {
        "**.h",
        "**.cpp"
    }

    defines
    {
        "NOMINMAX"
    }

    includedirs
    {
        "src",

        "%{wks.location}/vendor/OpenGL/include",
        "%{wks.location}/vendor/stdext/include",
        "%{wks.location}/vendor/ImGui/include",
    }

    libdirs
    {
        "%{wks.location}/vendor/ImGui/lib",
    }

    links
    {
        "ImGui"
    }

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    filter "system:windows"
        systemversion "latest"
        defines { "WINDOWS" }
        links
        {
            "OpenGL32",
            "wbemuuid",
            "dwmapi"
        }

    filter "configurations:Debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "RELEASE" }
        runtime "Release"
        optimize "On"