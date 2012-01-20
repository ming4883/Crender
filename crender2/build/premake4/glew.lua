-- A project defines one build target
project "glew"
	kind "StaticLib"
	language "C++"
	files { "../../src/glew/**.h", "../../src/glew/**.c" }
	-- excludes { "../../src/liblfds/driver_entry.c" }
	defines { "_CRT_SECURE_NO_WARNINGS", "GLEW_STATIC" }
	includedirs { "../../src/glew" }

	configuration "Debug"
		defines { "DEBUG" }
		flags { "Symbols" }

	configuration "Release"
		defines { "NDEBUG" }
		flags { "Optimize" }