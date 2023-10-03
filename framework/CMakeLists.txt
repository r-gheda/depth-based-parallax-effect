cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
project(cs4365 CXX)

# Slightly modified versions of the files from the cpp-starter-project by Jason Turner:
# https://github.com/lefticus/cpp_starter_project/
include("cmake/CompilerWarnings.cmake") # Enable almost all compiler warnings and CMake option to enable -Werror.
include("cmake/Sanitizers.cmake") # CMake options to enable address, memory, UB and thread sanitizers.
include("cmake/StaticAnalyzers.cmake") # CMake options to enable clang-tidy or cpp-check.

add_subdirectory("third_party")

# find_package(Threads REQUIRED) # For TBB
add_library(CGFramework STATIC
	"src/image.cpp"
)
target_include_directories(CGFramework PRIVATE "include/framework/" PUBLIC "include/")

target_link_libraries(CGFramework PUBLIC fmt stb glm)
# target_link_libraries(CGFramework PUBLIC fmt stb glm Threads::Threads TBB::tbb) # + TBB

target_compile_features(CGFramework PUBLIC cxx_std_20)

# Prevent accidentaly picking up a system-wide or vcpkg install of another loader (e.g. GLEW).
#target_compile_definitions(CGFramework PUBLIC "-DIMGUI_IMPL_OPENGL_LOADER_GLAD=1")
