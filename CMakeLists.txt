cmake_minimum_required(VERSION 3.28)

project(NightShift LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
include(FetchContent)

set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)

FetchContent_Declare(SFML
	GIT_REPOSITORY https://github.com/SFML/SFML.git
	GIT_TAG 3.1.0
	GIT_SHALLOW ON
	EXCLUDE_FROM_ALL
	SYSTEM
)

FetchContent_MakeAvailable(SFML)
add_executable(NightShift
    src/main.cpp
    src/Application.cpp
    src/Button.cpp
    src/MainMenu.cpp
)

target_include_directories(NightShift PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_features(NightShift PRIVATE cxx_std_17)

target_link_libraries(NightShift PRIVATE
	SFML::Graphics
	SFML::Audio
)
