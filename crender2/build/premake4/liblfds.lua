-- A project defines one build target
project "liblfds"
	kind "StaticLib"
	language "C++"
	files { "../../src/liblfds/**.h", "../../src/liblfds/**.c" }
	excludes { "../../src/liblfds/driver_entry.c" }
	defines { "_CRT_SECURE_NO_WARNINGS" }
	includedirs { "../../src/liblfds" }

	configuration "Debug"
		defines { "DEBUG" }
		flags { "Symbols" }

	configuration "Release"
		defines { "NDEBUG" }
		flags { "Optimize" }