cmake_minimum_required(VERSION 3.10)

set(MANAGERS_SOURCES
        ${SRC_DIR}/managers/JSONImporter.cpp
        ${HEADER_DIR}/managers/JSONImporter.hpp
        ${SRC_DIR}/managers/GLTFLoader.cpp
        ${HEADER_DIR}/managers/GLTFLoader.hpp
        ${SRC_DIR}/managers/ResourceManager.cpp
        ${HEADER_DIR}/managers/ResourceManager.hpp
        ${SRC_DIR}/managers/SceneManager.cpp
        ${HEADER_DIR}/managers/SceneManager.hpp
        ${SRC_DIR}/managers/FileManager.cpp
        ${HEADER_DIR}/managers/FileManager.hpp
)

add_library(managers STATIC ${MANAGERS_SOURCES})

target_link_libraries(managers PUBLIC render-resources)
