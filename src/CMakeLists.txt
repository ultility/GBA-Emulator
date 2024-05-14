message(STATUS "CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}")
add_executable(${PROJECT_NAME} main.c)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/display/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/cpu/include)
set("SDL2_ttf_DIR" "/usr/lib/x86_64-linux-gnu/cmake/SDL2_ttf/")
find_package(SDL2_ttf REQUIRED)
find_library(LibElf LibElf)
target_link_libraries(${PROJECT_NAME} LibDisplay LibCpu SDL2_ttf::SDL2_ttf ${LibElf})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/times.ttf DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/asm/bios.elf DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
add_definitions(-DTIMES_TTF_PATH="${CMAKE_CURRENT_BINARY_DIR}/times.ttf")
add_definitions(-DBIOS_PATH="${CMAKE_CURRENT_BINARY_DIR}/bios.elf")


