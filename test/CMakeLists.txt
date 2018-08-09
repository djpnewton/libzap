cmake_minimum_required(VERSION 3.0)

add_subdirectory(../src build)

set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++")
project(test_libzap)

set(SOURCE test.cpp)

add_executable(${PROJECT_NAME} ${SOURCE})
target_link_libraries(${PROJECT_NAME} zap waves_c)
