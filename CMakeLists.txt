cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
project(cs4365 CXX)

set(POISSON_MAIN_EXE "poisson")
set(BILATERAL_FILTER_MAIN_EXE "bilateral_filter")
set(PARALLAX_MAIN_EXE "parallax")

# Binaries directly to the binary dir without subfolders.
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_definitions("-fopenmp")
# add_definitions("-lstdc++fs")

if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/framework")
	# Create framework library and include CMake scripts (compiler warnings, sanitizers and static analyzers).
	add_subdirectory("framework") 
else() 
	# During development the framework lives in parent folder.
	add_subdirectory("../../../framework/" "${CMAKE_BINARY_DIR}/framework/")
endif()

add_executable(${POISSON_MAIN_EXE} "src/poisson_main.cpp" "src/helpers.h")

target_compile_features(${POISSON_MAIN_EXE} PRIVATE cxx_std_20)
target_link_libraries(${POISSON_MAIN_EXE} PRIVATE CGFramework)
enable_sanitizers(${POISSON_MAIN_EXE})
set_project_warnings(${POISSON_MAIN_EXE})

# OpenMP support.
find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND) 
    target_link_libraries(${POISSON_MAIN_EXE} PRIVATE OpenMP::OpenMP_CXX)
else()
	message( SEND_ERROR "OpenMP not found" )
endif()

# SET cwd for the MSVS debugger: https://stackoverflow.com/questions/41864259/how-to-set-working-directory-for-visual-studio-2017-rc-cmake-project
# set (VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}) 
set_target_properties(
	${POISSON_MAIN_EXE} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
	
	# Copy all files in the resources folder to the build directory after every successful build.
	# add_custom_command(TARGET ${POISSON_MAIN_EXE} POST_BUILD
	# 	COMMAND ${CMAKE_COMMAND} -E copy_directory
	# 	"${CMAKE_CURRENT_LIST_DIR}/data" $<TARGET_FILE_DIR:${POISSON_MAIN_EXE}>/data)
	
	
# Preprocessor definitions for path.
target_compile_definitions(${POISSON_MAIN_EXE} PRIVATE "-DDATA_DIR=\"${CMAKE_CURRENT_LIST_DIR}/data/\"" "-DOUTPUT_DIR=\"${CMAKE_CURRENT_LIST_DIR}/outputs\"")

add_executable(${BILATERAL_FILTER_MAIN_EXE} "src/bilateral_filter_main.cpp" "src/helpers.h")

target_compile_features(${BILATERAL_FILTER_MAIN_EXE} PRIVATE cxx_std_20)
target_link_libraries(${BILATERAL_FILTER_MAIN_EXE} PRIVATE CGFramework)
enable_sanitizers(${BILATERAL_FILTER_MAIN_EXE})
set_project_warnings(${BILATERAL_FILTER_MAIN_EXE})

# OpenMP support.
find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND) 
	target_link_libraries(${BILATERAL_FILTER_MAIN_EXE} PRIVATE OpenMP::OpenMP_CXX)
else()
	message( SEND_ERROR "OpenMP not found" )
endif()

# SET cwd for the MSVS debugger: https://stackoverflow.com/questions/41864259/how-to-set-working-directory-for-visual-studio-2017-rc-cmake-project
# set (VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}) 
set_target_properties(
	${BILATERAL_FILTER_MAIN_EXE} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
	
	# Copy all files in the resources folder to the build directory after every successful build.
	# add_custom_command(TARGET ${POISSON_MAIN_EXE} POST_BUILD
	# 	COMMAND ${CMAKE_COMMAND} -E copy_directory
	# 	"${CMAKE_CURRENT_LIST_DIR}/data" $<TARGET_FILE_DIR:${POISSON_MAIN_EXE}>/data)
	
	
# Preprocessor definitions for path.
target_compile_definitions(${BILATERAL_FILTER_MAIN_EXE} PRIVATE "-DDATA_DIR=\"${CMAKE_CURRENT_LIST_DIR}/data/\"" "-DOUTPUT_DIR=\"${CMAKE_CURRENT_LIST_DIR}/outputs\"")

add_executable(${PARALLAX_MAIN_EXE} "src/parallax_main.cpp" "src/helpers.h")

target_compile_features(${PARALLAX_MAIN_EXE} PRIVATE cxx_std_20)
target_link_libraries(${PARALLAX_MAIN_EXE} PRIVATE CGFramework)
enable_sanitizers(${PARALLAX_MAIN_EXE})
set_project_warnings(${PARALLAX_MAIN_EXE})

# OpenMP support.
find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND) 
	target_link_libraries(${PARALLAX_MAIN_EXE} PRIVATE OpenMP::OpenMP_CXX)
else()
	message( SEND_ERROR "OpenMP not found" )
endif()

# SET cwd for the MSVS debugger: https://stackoverflow.com/questions/41864259/how-to-set-working-directory-for-visual-studio-2017-rc-cmake-project
# set (VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}) 
set_target_properties(
	${PARALLAX_MAIN_EXE} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
	
	# Copy all files in the resources folder to the build directory after every successful build.
	# add_custom_command(TARGET ${POISSON_MAIN_EXE} POST_BUILD
	# 	COMMAND ${CMAKE_COMMAND} -E copy_directory
	# 	"${CMAKE_CURRENT_LIST_DIR}/data" $<TARGET_FILE_DIR:${POISSON_MAIN_EXE}>/data)
	
	
# Preprocessor definitions for path.
target_compile_definitions(${PARALLAX_MAIN_EXE} PRIVATE "-DDATA_DIR=\"${CMAKE_CURRENT_LIST_DIR}/data/\"" "-DOUTPUT_DIR=\"${CMAKE_CURRENT_LIST_DIR}/outputs\"")

if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/grading_tests/")
	add_subdirectory("grading_tests")
endif()	

