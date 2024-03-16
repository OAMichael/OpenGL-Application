cmake_minimum_required(VERSION 3.10)

set(RENDER_SOURCES
        ${HEADER_DIR}/renderResources/RenderResource.hpp
        ${SRC_DIR}/renderResources/Shader.cpp
        ${HEADER_DIR}/renderResources/Shader.hpp
        ${SRC_DIR}/renderResources/Image.cpp
        ${HEADER_DIR}/renderResources/Image.hpp
        ${SRC_DIR}/renderResources/Sampler.cpp
        ${HEADER_DIR}/renderResources/Sampler.hpp
        ${SRC_DIR}/renderResources/Texture.cpp
        ${HEADER_DIR}/renderResources/Texture.hpp
        ${SRC_DIR}/renderResources/Material.cpp
        ${HEADER_DIR}/renderResources/Material.hpp
        ${SRC_DIR}/renderResources/Buffer.cpp
        ${HEADER_DIR}/renderResources/Buffer.hpp
        ${SRC_DIR}/renderResources/Framebuffer.cpp
        ${HEADER_DIR}/renderResources/Framebuffer.hpp
)

add_library(render-resources STATIC ${RENDER_SOURCES})
