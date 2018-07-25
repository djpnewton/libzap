cmake_minimum_required(VERSION 3.0)

add_subdirectory(../waves-c build)

project(zap)

set(SOURCE zap.c zap_jni.c)

add_library(${PROJECT_NAME} SHARED ${SOURCE})
target_link_libraries(${PROJECT_NAME} waves_c)
