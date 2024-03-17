cmake_minimum_required(VERSION 3.10)

project(OpenGL VERSION 1.0)
cmake_policy(SET CMP0076 NEW)

set(CMAKE_CXX_STANDARD 17)

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(HEADER_DIR ${CMAKE_SOURCE_DIR}/headers)
set(INCLUDE_DIR
		${CMAKE_SOURCE_DIR}/headers
		${CMAKE_SOURCE_DIR}/3rdparty
		${CMAKE_SOURCE_DIR}/3rdparty/freetype
)
set(LIB_DIR ${CMAKE_SOURCE_DIR}/3rdparty/lib)

include_directories(
		${SRC_DIR}/app
		${SRC_DIR}/renderResources
		${SRC_DIR}/managers
		${SRC_DIR}/scene
		${HEADER_DIR}/app
		${HEADER_DIR}/renderResources
		${HEADER_DIR}/managers
		${HEADER_DIR}/scene
		${HEADER_DIR}/utils
		${CMAKE_SOURCE_DIR}/3rdparty
		${CMAKE_SOURCE_DIR}/3rdparty/glad
		${CMAKE_SOURCE_DIR}/3rdparty/freetype
)

link_directories(${LIB_DIR})


set(GLSL_VERSION "460 core")

if(ANDROID)
	set(GLSL_VERSION "320 es")
	include_directories(${ANDROID_NDK}/sources/android/native_app_glue)
endif()

configure_file(${CMAKE_SOURCE_DIR}/shaders/GLSLversion.h.in ${CMAKE_SOURCE_DIR}/shaders/GLSLversion.h @ONLY)

add_subdirectory(3rdparty)
add_subdirectory(src)

if(ANDROID)
	add_subdirectory(android/app/src/main/cpp)
endif()
