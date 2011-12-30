-- A project defines one build target
project "unittest++"
	kind "StaticLib"
	language "C++"
	files { "../../src/unittest++/**.h", "../../src/unittest++/**.cpp" }
	excludes { "../../src/unittest++/Posix/**" }
	defines { "_CRT_SECURE_NO_WARNINGS" }

	configuration "Debug"
		defines { "DEBUG" }
		flags { "Symbols" }

	configuration "Release"
		defines { "NDEBUG" }
		flags { "Optimize" }