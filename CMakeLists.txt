cmake_minimum_required(VERSION 3.0)

add_subdirectory(waves-c)

project(test_wavesc_lib)

set(SOURCE src/test.c)

add_executable(${PROJECT_NAME} ${SOURCE})
target_link_libraries(${PROJECT_NAME} waves_c)