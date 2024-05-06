add_library(LibDisplay display.c)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(LibDisplay ${SDL2_LIBRARIES})