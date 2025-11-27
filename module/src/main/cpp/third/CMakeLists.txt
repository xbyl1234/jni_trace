cmake_minimum_required(VERSION 3.28.0+)
project("third")
enable_language(ASM)

file(GLOB utils_src
        "utils/*.h"
        "utils/*.cpp"
        "utils/*.c"
        "utils/*.asm"
)

add_library(libutils
        STATIC
        ${utils_src})
