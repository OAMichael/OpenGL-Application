cmake_minimum_required(VERSION 3.10)

project(OpenGL VERSION 1.0)
cmake_policy(SET CMP0076 NEW)

set(CMAKE_CXX_STANDARD 17)
set(SRC_DIR ./src)
set(GLAD_DIR ./3rdparty/glad)
set(INCLUDE_DIR ./headers ./3rdparty)
set(COMPILE_OPT -std=c++17 -O2)
set(LIB_DIR ./3rdparty/lib)

if(MSVC)
	set(LINK_OPT opengl32.lib glfw3.lib)
else()
	set(LINK_OPT -lglfw -lGL -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi)
endif()

add_executable(
	App.exe ${SRC_DIR}/App.cpp
	${GLAD_DIR}/glad.c
	${SRC_DIR}/IApplication.cpp
	${SRC_DIR}/Shader.cpp
	${SRC_DIR}/PotentialApp.cpp
	${SRC_DIR}/Camera.cpp
	${SRC_DIR}/Cube.cpp
	${SRC_DIR}/ISceneObject.cpp
	${SRC_DIR}/Model.cpp
	${SRC_DIR}/GLTFLoader.cpp
	${SRC_DIR}/Mesh.cpp
	${SRC_DIR}/SceneNode.cpp
	${SRC_DIR}/Image.cpp
	${SRC_DIR}/Texture.cpp
	${SRC_DIR}/ResourceManager.cpp
	${SRC_DIR}/Material.cpp
)

target_include_directories(App.exe PUBLIC ${SRC_DIR})
target_include_directories(App.exe PUBLIC ${GLAD_DIR})

include_directories(${INCLUDE_DIR})
link_directories(${LIB_DIR})

if(MSVC)
	set(CMAKE_VS_SDK_INCLUDE_DIRECTORIES $(IncludePath) ${INCLUDE_DIR})
	set(CMAKE_VS_SDK_LIBRARY_DIRECTORIES $(LibraryPath) ${LIB_DIR})
endif()

add_compile_options(App.exe ${COMPILE_OPT})
target_link_libraries(App.exe PUBLIC ${LINK_OPT})
target_link_directories(App.exe PUBLIC ${LIB_DIR})