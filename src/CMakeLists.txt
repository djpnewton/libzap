cmake_minimum_required(VERSION 3.0)

add_subdirectory(../waves-c build)

project(zap)

set(SOURCE zap.c)

if (ANDROID)
    set(SOURCE ${SOURCE} zap_jni.c)
    include_directories(${CURL_INCLUDE_DIR})
    set(ZLIB_LIBRARY "-lz")
    find_package(ZLIB REQUIRED) 
endif()

set(CURL_LIBRARY "-lcurl") 
find_package(CURL REQUIRED) 
add_library(${PROJECT_NAME} SHARED ${SOURCE})
target_link_libraries(${PROJECT_NAME} waves_c ${CURL_LIBRARIES} ${ZLIB_LIBRARIES})
