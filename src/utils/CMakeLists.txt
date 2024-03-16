cmake_minimum_required(VERSION 3.10)

set(UTILS_SOURCES
        ${SRC_DIR}/utils/Logger.cpp
        ${HEADER_DIR}/utils/Logger.hpp
)

add_library(utils STATIC ${UTILS_SOURCES})
