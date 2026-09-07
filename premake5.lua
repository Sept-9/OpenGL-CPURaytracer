workspace "COMP5892M-rt"
	language "C++"
	cppdialect "C++20"

	platforms { "x64" }
	configurations { "debug", "release" }

	flags "NoPCH"
	flags "MultiProcessorCompile"

	startproject "main"

	debugdir "%{wks.location}"
	objdir "_build_/%{cfg.buildcfg}-%{cfg.platform}-%{cfg.toolset}"
	targetsuffix "-%{cfg.buildcfg}-%{cfg.platform}-%{cfg.toolset}"
	
	-- Default toolset options
	filter "toolset:gcc or toolset:clang"
		linkoptions { "-pthread" }
		buildoptions { "-march=native", "-Wall", "-pthread" }

		-- The -ffast-math option is a bit sketchy, as it permits the compiler
		-- to violate some of the standard rules around floating point math in
		-- favour of better optimizations. It _will_ ruin code that has been 
		-- carefully tweaked for precision. However, this code hasn't, so this
		-- makes for a nice speed-up.
		buildoptions{ "-ffast-math" }

	filter "toolset:msc-*"
		defines { "_CRT_SECURE_NO_WARNINGS=1" }
		defines { "_SCL_SECURE_NO_WARNINGS=1" }
		buildoptions { "/utf-8" }
	
	filter "*"

	-- default libraries
	filter "system:linux"
		links "dl"
		links "gomp"
		links "GL"
		links "GLX"
	
	filter "system:windows"
		links "opengl32"

	filter "*"

	-- default outputs
	filter "kind:StaticLib"
		targetdir "lib/"

	filter "kind:ConsoleApp"
		targetdir "bin/"
		targetextension ".exe"
	
	filter "*"

	--configurations
	filter "debug"
		symbols "On"
		defines { "_DEBUG=1" }

	filter "release"
		optimize "On"
		defines { "NDEBUG=1" }

		-- The code doesn't use inlining, so it misses out on a ton (really!)
		-- optimizations from the compiler. Enabling LTO brings back some of
		-- those. It makes for a 3+x speedup on GCC 13.
		flags "LinkTimeOptimization"


	filter "*"

-- Third party dependencies
include "external" 

-- Projects
project "main"
	local sources = { 
		"src/**.cpp",
		"src/**.h",
	}

	kind "ConsoleApp"
	location "src"
	
	openmp "on"

	files( sources )

	links "x-glfw"
	links "x-glew"

	includedirs( "." );


--EOF
