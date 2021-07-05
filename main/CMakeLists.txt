cmake_minimum_required(VERSION 3.7)

project( LuaTutorial )

add_compile_options(-W -Wall -Werror) #All Warnings, all warnings are errors

# source for the test executable
set  (LUA_TUTORIAL_SOURCES
		"main.cpp" )		
        
source_group("src" FILES ${LUA_TUTORIAL_SOURCES})
		
add_executable( LuaTutorial
	${LUA_TUTORIAL_SOURCES} 
	)

target_link_libraries( LuaTutorial PUBLIC LuaLib )

#find_package(RTTR CONFIG REQUIRED Core)
#target_link_libraries(LuaTutorial PUBLIC RTTR::Core_Lib)     # rttr as static library
