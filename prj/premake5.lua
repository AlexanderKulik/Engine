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
	
	
	project "jsoncpp"
		kind "StaticLib"
		includedirs {
			"../libs/jsoncpp/include",
		}
		files {
			"../libs/jsoncpp/include/**",
			"../libs/jsoncpp/src/lib_json/**"
		}
	
	project "EngineLib"
		kind "StaticLib"
		includedirs {
			"../libs/boost",
			"../libs/jsoncpp/include",
		}
		files "../src/EngineLib/**"
		links {
			"jsoncpp"
		}
	
	project "EngineApp"
		kind "WindowedApp"
		files "../src/EngineApp/**"
		includedirs "../src/EngineLib"
		links "EngineLib"
	
	
	group "Tests"
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