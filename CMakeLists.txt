cmake_minimum_required(VERSION 3.10)

project(OpenGL VERSION 1.0)
cmake_policy(SET CMP0076 NEW)

set(CMAKE_CXX_STANDARD 17)
set(SRC_DIR ./src)
set(GLAD_DIR ./external)
set(COMPILE_OPT -std=c++17 -O2)
set(LINK_OPT -lglfw -lGL -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi)

add_executable(App.exe ${SRC_DIR}/App.cpp ${GLAD_DIR}/glad.c)

target_include_directories(App.exe PUBLIC ${SRC_DIR})
target_include_directories(App.exe PUBLIC ${GLAD_DIR})

add_library(Application ${SRC_DIR}/Application.cpp)
target_link_libraries(App.exe PUBLIC Application)

add_library(Shader ${SRC_DIR}/Shader.cpp)
target_link_libraries(App.exe PUBLIC Shader)

add_library(PotentialApp ${SRC_DIR}/PotentialApp.cpp)
target_link_libraries(App.exe PUBLIC PotentialApp)

add_library(Camera ${SRC_DIR}/Camera.cpp)
target_link_libraries(App.exe PUBLIC Camera)

add_library(Cube ${SRC_DIR}/Cube.cpp)
target_link_libraries(App.exe PUBLIC Cube)

add_library(SceneObject ${SRC_DIR}/SceneObject.cpp)
target_link_libraries(App.exe PUBLIC SceneObject)

add_compile_options(App.exe ${COMPILE_OPT})
target_link_libraries(App.exe PUBLIC ${LINK_OPT})