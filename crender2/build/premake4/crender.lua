-- A project defines one build target
project "crender"
	kind "StaticLib"
	language "C++"
	files { "../../src/core/**.h", "../../src/core/**.c", "../../src/core/**.cpp" }
	includedirs { "../../src/glew" }
	defines { "_CRT_SECURE_NO_WARNINGS", "GLEW_STATIC", "NO_NED_NAMESPACE" }
	links { "glew" }

	configuration "Debug"
		defines { "DEBUG" }
		flags { "Symbols" }

	configuration "Release"
		defines { "NDEBUG" }
		flags { "Optimize" }

project "crender_core_test"
	kind "ConsoleApp"
	language "C++"
	files { "../../test/main.cpp", "../../test/core/**.cpp" }
	includedirs { "../../src/core", "../../src/unittest-cpp" }
	links { "unittest-cpp", "crender" }
	
	configuration "Debug"
		defines { "DEBUG" }
		flags { "Symbols" }

	configuration "Release"
		defines { "NDEBUG" }
		flags { "Optimize" }
		
project "crender_framework_test"
	kind "ConsoleApp"
	language "C++"
	files { "../../src/framework/**.h", "../../src/framework/**.c", "../../src/framework/**.cpp" , "../../test/framework/**.cpp" }
	includedirs { "../../src/core", "../../src/framework", "../../src" }
	defines { "_CRT_SECURE_NO_WARNINGS" }
	links { "crender", "opengl32", "glu32" }
	
	configuration "Debug"
		defines { "DEBUG" }
		flags { "Symbols" }

	configuration "Release"
		defines { "NDEBUG" }
		flags { "Optimize" }