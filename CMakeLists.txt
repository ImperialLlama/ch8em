cmake_minimum_required(VERSION 3.14)
project(ch8em C)

set(CMAKE_C_STANDARD 99)

include(FindPkgConfig)
pkg_search_module(SDL2 REQUIRED sdl2)

include_directories(${SDL2_INCLUDE_DIRS})

add_executable(ch8em
        src/main.c
        src/chip8.c
        src/chip8.h src/macros.h src/output.c src/output.h
        src/chip8_instruction.h src/chip8_instruction.c src/opcode.h)
target_link_libraries(ch8em ${SDL2_LIBRARIES})
