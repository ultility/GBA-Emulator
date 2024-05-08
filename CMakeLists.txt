cmake_minimum_required(VERSION 3.14)
project(GBA-Emulator VERSION 0.1.0 LANGUAGES C)

add_subdirectory(display)
add_subdirectory(cpu)
include_directories(include)
add_subdirectory(src)