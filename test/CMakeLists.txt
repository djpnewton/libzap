cmake_minimum_required(VERSION 3.0)

add_subdirectory(../src build)

project(test_libzap)

set(SOURCE test.c)

add_executable(${PROJECT_NAME} ${SOURCE})
target_link_libraries(${PROJECT_NAME} zap waves_c)
