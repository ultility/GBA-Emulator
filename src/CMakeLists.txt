
add_executable(${PROJECT_NAME} main.c)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/display/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/cpu/include)

find_package(SDL2_ttf CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} LibDisplay LibCpu SDL2_ttf::SDL2_ttf)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/times.ttf DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
add_definitions(-DTIMES_TTF_PATH="${CMAKE_CURRENT_BINARY_DIR}/times.ttf")


