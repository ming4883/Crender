-- A project defines one build target
project "unittest-cpp"
	kind "StaticLib"
	language "C++"
	files { "../../src/unittest-cpp/**.h", "../../src/unittest-cpp/**.cpp" }
	-- excludes { "../../src/unittest-cpp/Posix/**" }
	excludes { "../../src/unittest-cpp/Win32/**" }
	defines { "_CRT_SECURE_NO_WARNINGS" }

	configuration "Debug"
		defines { "DEBUG" }
		flags { "Symbols" }

	configuration "Release"
		defines { "NDEBUG" }
		flags { "Optimize" }