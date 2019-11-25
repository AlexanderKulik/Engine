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
	
	debugdir ("../build")
	targetdir ("../build/bin/%{cfg.longname}")
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
		
	project "DirectXTK"
		kind "StaticLib"
		files {
			"../libs/DirectXTK/Inc/**",
			"../libs/DirectXTK/Src/**",
		}
		excludes {
			"../libs/DirectXTK/Inc/XboxDDSTextureLoader.h",
			"../libs/DirectXTK/Src/XboxDDSTextureLoader.cpp",
			"../libs/DirectXTK/Src/Shaders/**",
		}
		includedirs "../libs/DirectXTK/Inc"
		pchheader "pch.h"
		pchsource "../libs/DirectXTK/Src/pch.cpp"
	
	project "EngineApp"
		kind "WindowedApp"
		flags { "FatalCompileWarnings" }
		files "../src/EngineApp/source/**"
		includedirs {
			"../libs/boost",
			"../src/EngineLib/source",
			"../libs/DirectXTK/Inc"
		}
		links {
			"d3d11",
			"d3dcompiler",
			"EngineLib",
			"DirectXTK",
		}
		pchheader "pch.h"
		pchsource "../src/EngineApp/source/pch.cpp"
	
	
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