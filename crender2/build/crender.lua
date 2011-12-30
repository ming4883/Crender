-- A project defines one build target
project "crender"
	kind "StaticLib"
	language "C++"
	files { "../../src/core/**.h", "../../src/core/**.c", "../../src/core/**.cpp" }

	configuration "Debug"
		defines { "DEBUG" }
		flags { "Symbols" }

	configuration "Release"
		defines { "NDEBUG" }
		flags { "Optimize" }