cmake_minimum_required(VERSION 3.10)

set(SCENE_SOURCES
        ${SRC_DIR}/scene/Camera.cpp
        ${HEADER_DIR}/scene/Camera.hpp
        ${SRC_DIR}/scene/Cube.cpp
        ${HEADER_DIR}/scene/Cube.hpp
        ${SRC_DIR}/scene/ISceneObject.cpp
        ${HEADER_DIR}/scene/ISceneObject.hpp
        ${SRC_DIR}/scene/SceneNode.cpp
        ${HEADER_DIR}/scene/SceneNode.hpp
        ${SRC_DIR}/scene/Mesh.cpp
        ${HEADER_DIR}/scene/Mesh.hpp
        ${SRC_DIR}/scene/Model.cpp
        ${HEADER_DIR}/scene/Model.hpp
        ${HEADER_DIR}/scene/Light.hpp
)

add_library(scene-resources STATIC ${SCENE_SOURCES})
