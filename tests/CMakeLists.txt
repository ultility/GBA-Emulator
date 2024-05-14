include(CheckLibraryExists)
find_library(LibCpu LibCpu PATHS ${CMAKE_BINARY_DIR}/cpu/src)
find_library(MATH_LIBRARY m)
find_package(Check REQUIRED)

add_executable(test_cpu check_cpu.c)
target_include_directories(test_cpu PRIVATE ${CMAKE_SOURCE_DIR}/cpu/include)
target_link_libraries(test_cpu LibCpu ${CHECK_LIBRARIES} ${MATH_LIBRARY} pthread subunit)

enable_testing()

add_test(NAME test_cpu COMMAND test_cpu WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests) 