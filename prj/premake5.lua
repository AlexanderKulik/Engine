workspace "Engine"
	
	location "Generated"
	language "C++"
	architecture "x86_64"
	configurations { "Debug", "Release" }
	
	filter { "configurations:Debug" }
		symbols "On"
	
	filter { "configurations:Release" }
		optimize "On"
	
	filter { }
	
	targetdir ("Build/Bin/%{prj.name}/%{cfg.longname}")
	objdir ("Build/Obj/%{prj.name}/%{cfg.longname}")
	
	
	project "gtest"
		kind "StaticLib"
		includedirs {
			"../libs/gtest/googletest/include",
			"../libs/gtest/googletest"
		}
		files "../libs/gtest/googletest/src/gtest-all.cc"
	
	
	project "gtest_main"
		kind "StaticLib"
		includedirs {
			"../libs/gtest/googletest/include",
			"../libs/gtest/googletest"
		}
		files "../libs/gtest/googletest/src/gtest_main.cc"
	
	
	project "EngineLib"
		kind "StaticLib"
		files "../src/EngineLib/**"
	
	
	project "EngineApp"
		kind "ConsoleApp"
		files "../src/EngineApp/**"
		includedirs "../src/EngineLib"
		links "EngineLib"
	
	
	project "UnitTests"
		kind "ConsoleApp"
		files "../test/**"
		includedirs {
			"../libs/gtest/googletest/include",
			"../src/EngineLib"
		}
		links {
			"gtest",
			"gtest_main",
			"EngineLib"
		}