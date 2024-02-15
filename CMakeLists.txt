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

if(WIN32)
	set(LINK_OPT opengl32.lib glfw3.lib)
else()
	set(LINK_OPT -lglfw -lGL -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi)
endif()

add_executable(App

			${SRC_DIR}/app/App.cpp
			${SRC_DIR}/app/IApplication.cpp
			${SRC_DIR}/app/PotentialApp.cpp

			${SRC_DIR}/renderResources/Shader.cpp
			${SRC_DIR}/renderResources/Image.cpp
			${SRC_DIR}/renderResources/Sampler.cpp
			${SRC_DIR}/renderResources/Texture.cpp
			${SRC_DIR}/renderResources/Material.cpp
			${SRC_DIR}/renderResources/Buffer.cpp
			${SRC_DIR}/renderResources/Framebuffer.cpp

			${SRC_DIR}/managers/GLTFLoader.cpp
			${SRC_DIR}/managers/ResourceManager.cpp
			${SRC_DIR}/managers/SceneManager.cpp
			${SRC_DIR}/managers/JSONImporter.cpp

			${SRC_DIR}/scene/Camera.cpp
			${SRC_DIR}/scene/Cube.cpp
			${SRC_DIR}/scene/ISceneObject.cpp
			${SRC_DIR}/scene/SceneNode.cpp
			${SRC_DIR}/scene/Mesh.cpp
			${SRC_DIR}/scene/Model.cpp

			${HEADER_DIR}/app/IApplication.hpp
			${HEADER_DIR}/app/PotentialApp.hpp

			${HEADER_DIR}/renderResources/RenderResource.hpp
			${HEADER_DIR}/renderResources/Shader.hpp
			${HEADER_DIR}/renderResources/Image.hpp
			${HEADER_DIR}/renderResources/Sampler.hpp
			${HEADER_DIR}/renderResources/Texture.hpp
			${HEADER_DIR}/renderResources/Material.hpp
			${HEADER_DIR}/renderResources/Buffer.hpp
			${HEADER_DIR}/renderResources/Framebuffer.hpp

			${HEADER_DIR}/managers/GLTFLoader.hpp
			${HEADER_DIR}/managers/ResourceManager.hpp
			${HEADER_DIR}/managers/SceneManager.hpp
			${HEADER_DIR}/managers/JSONImporter.hpp

			${HEADER_DIR}/scene/Camera.hpp
			${HEADER_DIR}/scene/Cube.hpp
			${HEADER_DIR}/scene/ISceneObject.hpp
			${HEADER_DIR}/scene/SceneNode.hpp
			${HEADER_DIR}/scene/Mesh.hpp
			${HEADER_DIR}/scene/Model.hpp
			${HEADER_DIR}/scene/Light.hpp

			${GLAD_DIR}/glad.c
)

target_include_directories(App PUBLIC ${INCLUDE_DIR} ${SRC_DIR} ${GLAD_DIR})
target_link_directories(App PUBLIC ${LIB_DIR})
target_compile_options(App PUBLIC ${COMPILE_OPT})
target_link_libraries(App PUBLIC ${LINK_OPT})

target_include_directories(App PUBLIC
						   ${SRC_DIR}/app
						   ${SRC_DIR}/renderResources
						   ${SRC_DIR}/managers
						   ${SRC_DIR}/scene
						   ${HEADER_DIR}/app
						   ${HEADER_DIR}/renderResources
						   ${HEADER_DIR}/managers
						   ${HEADER_DIR}/scene
)


if(MSVC)
	set(CMAKE_VS_SDK_INCLUDE_DIRECTORIES $(IncludePath) ${INCLUDE_DIR})
	set(CMAKE_VS_SDK_LIBRARY_DIRECTORIES $(LibraryPath) ${LIB_DIR})
endif()
