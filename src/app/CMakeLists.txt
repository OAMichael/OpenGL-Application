cmake_minimum_required(VERSION 3.10)

set(APPLICATION_SOURCES
        ${SRC_DIR}/app/IApplication.cpp
        ${HEADER_DIR}/app/IApplication.hpp
        ${SRC_DIR}/app/PotentialApp.cpp
        ${HEADER_DIR}/app/PotentialApp.hpp
)

add_library(application STATIC ${APPLICATION_SOURCES})


if(NOT ANDROID)
    if(WIN32)
        set(LINK_OPT opengl32.lib glfw3.lib freetype.lib)
    else()
        set(LINK_OPT -lglfw -lGL -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi -lfreetype)
    endif()

    if (NOT MSVC OR (CMAKE_BUILD_TYPE EQUAL Release))
        set(COMPILE_OPT -O2)
    endif()

    add_executable(App App.cpp)
    target_include_directories(App PUBLIC ${INCLUDE_DIR} ${SRC_DIR} ${GLAD_DIR})
    target_link_directories(App PUBLIC ${LIB_DIR})
    target_compile_options(App PUBLIC ${COMPILE_OPT})
    target_link_libraries(App PUBLIC ${LINK_OPT})

    target_link_libraries(App PUBLIC
            application
            scene-resources
            render-resources
            managers
            utils
            glad-lib
    )

    if(MSVC)
        set(CMAKE_VS_SDK_INCLUDE_DIRECTORIES $(IncludePath) ${INCLUDE_DIR})
        set(CMAKE_VS_SDK_LIBRARY_DIRECTORIES $(LibraryPath) ${LIB_DIR})
    endif()
endif()
