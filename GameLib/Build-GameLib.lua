project "GameLib"
    kind "StaticLib"
    language "C++"
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
        "%{wks.location}/vendor/ImGuizmo",
        "%{wks.location}/vendor/stb",
        "%{wks.location}/vendor/assimp-5.4.3/include",
    }

    libdirs
    {
        "%{wks.location}/vendor/ImGui/lib",
        "%{wks.location}/vendor/assimp-5.4.3/lib",
    }

    links
    {
        "OpenGL32",
        "ImGui"
    }

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    postbuildcommands
    {
        "{COPYDIR} %{wks.location}/Common-DLLs %{wks.location}/bin/" .. outputdir .. "/Game"
    }

    filter "system:windows"
        systemversion "latest"
        defines { "WINDOWS" }
        links
        {
            "wbemuuid"
        }

    filter "configurations:Debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "On"
        links
        {
            "assimp-vc143-mtd.lib"
        }

    filter "configurations:Release"
        defines { "RELEASE" }
        runtime "Release"
        optimize "On"
        links
        {
            "assimp-vc143-mt.lib"
        }