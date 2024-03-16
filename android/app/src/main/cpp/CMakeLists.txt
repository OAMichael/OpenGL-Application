cmake_minimum_required(VERSION 3.10)

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

add_library(native-activity SHARED
        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
        main.cpp)

target_link_libraries(native-activity PUBLIC
        android
        EGL
        GLESv3
        log
)

target_link_libraries(native-activity PUBLIC
        application
        scene-resources
        render-resources
        managers
        utils
)