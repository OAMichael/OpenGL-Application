cmake_minimum_required(VERSION 3.10)

add_library(glad-lib STATIC glad/glad.c)

if(UNIX)
    target_link_libraries(glad-lib PUBLIC dl)
endif()
