cmake_minimum_required(VERSION 3.10)

project(OpenGL VERSION 1.0)
cmake_policy(SET CMP0076 NEW)

set(CMAKE_CXX_STANDARD 17)
set(SRC_DIR ./src)
set(HEADER_DIR ./headers)
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
	App

	${GLAD_DIR}/glad.c
	${SRC_DIR}/App.cpp
	${SRC_DIR}/IApplication.cpp
	${SRC_DIR}/Shader.cpp
	${SRC_DIR}/PotentialApp.cpp
	${SRC_DIR}/Camera.cpp
	${SRC_DIR}/Cube.cpp
	${SRC_DIR}/ISceneObject.cpp
	${SRC_DIR}/GLTFLoader.cpp
	${SRC_DIR}/SceneNode.cpp
	${SRC_DIR}/Image.cpp
	${SRC_DIR}/Sampler.cpp
	${SRC_DIR}/Texture.cpp
	${SRC_DIR}/Material.cpp
	${SRC_DIR}/Buffer.cpp
	${SRC_DIR}/Mesh.cpp
	${SRC_DIR}/Model.cpp
	${SRC_DIR}/ResourceManager.cpp
	${SRC_DIR}/SceneManager.cpp
	${SRC_DIR}/JSONImporter.cpp

	${HEADER_DIR}/IApplication.hpp
	${HEADER_DIR}/Shader.hpp
	${HEADER_DIR}/PotentialApp.hpp
	${HEADER_DIR}/Camera.hpp
	${HEADER_DIR}/Cube.hpp
	${HEADER_DIR}/ISceneObject.hpp
	${HEADER_DIR}/GLTFLoader.hpp
	${HEADER_DIR}/SceneNode.hpp
	${HEADER_DIR}/Image.hpp
	${HEADER_DIR}/Sampler.hpp
	${HEADER_DIR}/Texture.hpp
	${HEADER_DIR}/Material.hpp
	${HEADER_DIR}/Buffer.hpp
	${HEADER_DIR}/Mesh.hpp
	${HEADER_DIR}/Model.hpp
	${HEADER_DIR}/ResourceManager.hpp
	${HEADER_DIR}/SceneManager.hpp
	${HEADER_DIR}/JSONImporter.hpp
	${HEADER_DIR}/RenderResource.hpp
)

target_include_directories(App PUBLIC ${SRC_DIR})
target_include_directories(App PUBLIC ${GLAD_DIR})

include_directories(${INCLUDE_DIR})
link_directories(${LIB_DIR})

if(MSVC)
	set(CMAKE_VS_SDK_INCLUDE_DIRECTORIES $(IncludePath) ${INCLUDE_DIR})
	set(CMAKE_VS_SDK_LIBRARY_DIRECTORIES $(LibraryPath) ${LIB_DIR})
endif()

add_compile_options(App ${COMPILE_OPT})
target_link_libraries(App PUBLIC ${LINK_OPT})
target_link_directories(App PUBLIC ${LIB_DIR})