
cmake_minimum_required(VERSION 3.22.1)

project(gsv_cpu_threads LANGUAGES CXX)

# 主 JNI 动态库：保留原有 CPU 线程功能，加入 OpenJTalk JNI
add_library(
    gsv_cpu_threads
    SHARED
    cpu_threads.cpp
    openjtalk_jni.cpp
)

# 根据当前 Android ABI 定位 OpenJTalk 原生库
set(
    OPENJTALK_NATIVE_LIB
    "${CMAKE_CURRENT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libopenjtalk_native.so"
)

if(NOT EXISTS "${OPENJTALK_NATIVE_LIB}")
    message(FATAL_ERROR
        "OpenJTalk library not found: ${OPENJTALK_NATIVE_LIB}"
    )
endif()

# 声明预编译的 OpenJTalk 动态库
add_library(openjtalk_native SHARED IMPORTED)

set_target_properties(
    openjtalk_native
    PROPERTIES
    IMPORTED_LOCATION "${OPENJTALK_NATIVE_LIB}"
)

# 链接 OpenJTalk 和 Android 动态加载库
target_link_libraries(
    gsv_cpu_threads
    PRIVATE
    openjtalk_native
    dl
)
