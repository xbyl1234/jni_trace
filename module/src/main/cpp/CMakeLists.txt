cmake_minimum_required(VERSION 3.28.0+)
project("jni_trace")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../libs/${ANDROID_ABI}")
set(CXX_FLAGS "${CXX_FLAGS} -std=c++20 -fno-exceptions -fno-rtti -fvisibility=hidden -fvisibility-inlines-hidden")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_FLAGS}")

enable_language(ASM)

find_library(log-lib log)
find_package(cxx REQUIRED CONFIG)
link_libraries(cxx::cxx)

add_subdirectory(third/log2file)
add_subdirectory(./third)

file(GLOB analyse_src
        "*.h"
        "*.hpp"
        "*.cpp"
)

add_library(
        analyse
        SHARED
        ${analyse_src}
)

target_link_libraries(
        analyse
        libutils
        log2file
        ${log-lib}
)