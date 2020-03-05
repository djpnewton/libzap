cmake_minimum_required(VERSION 3.14)

# Creates C resources file from files in given directory
function(create_resources dir output)
    # Create empty output file
    file(WRITE ${output} "")
    # Collect input files
    file(GLOB bins ${dir}/*)
    # Iterate through input files
    foreach(bin ${bins})
        # Get short filename
        string(REGEX MATCH "([^/]+)$" filename ${bin})
        # Replace filename spaces & extension separator for C compatibility
        string(REGEX REPLACE "\\.| |-" "_" filename ${filename})
        # Read hex data from file
        file(READ ${bin} filedata HEX)
        # Convert hex data for C compatibility
        string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," filedata ${filedata})
        # Append data to output file
        file(APPEND ${output} "const unsigned char ${filename}[] = {${filedata}};\nconst unsigned ${filename}_size = sizeof(${filename});\n")
    endforeach()
endfunction()

add_subdirectory(../deps/waves-c build/waves-c)
add_subdirectory(../deps/jansson-2.12 build/jansson)

project(zap)

set(BIP39 ../deps/bip39/)
set(BIP39_SOURCE ${BIP39}/bip39.c ${BIP39}/hmac.c ${BIP39}/rand.c ${BIP39}/sha2.c ${BIP39}/pbkdf2.c ${BIP39}/memzero.c)

set(SOURCE zap.c)
set(LIBRARY_TYPE SHARED)
if (BUILD_STATIC)
    set(LIBRARY_TYPE STATIC)
endif()

if (ANDROID)
    create_resources(resources resources.c)
    set(SOURCE ${SOURCE} zap_jni.c)
    set(LOG_LIBRARIES log)
    set(ZLIB_LIBRARY "-lz")
    find_package(ZLIB REQUIRED)
endif()
if (IOS)
    add_subdirectory(../deps/zlib build/zlib)
    set(ZLIB_LIBRARIES z)
    set(CURL_LIBRARIES curl)
    set(SECURITY_LIBRARIES "-framework Security")
endif()
if (ANDROID OR IOS)
    include_directories(${CURL_INCLUDE_DIR})
    include_directories(../deps/jansson-2.12/src)
    include_directories(../deps/jansson-2.12/android)
    # use prebuilt curl
    add_library(curl STATIC IMPORTED SHARED)
    set_target_properties(curl PROPERTIES IMPORTED_LOCATION 
        ${CURL_LIBRARY})
    set(CURL_LIBRARIES "curl") 
else()
    # use system curl
    set(CURL_LIBRARY "-lcurl") 
    find_package(CURL REQUIRED) 
endif()

add_library(${PROJECT_NAME} ${LIBRARY_TYPE} ${SOURCE} ${BIP39_SOURCE})
target_link_libraries(${PROJECT_NAME} waves_c jansson ${ZLIB_LIBRARIES} curl ${CURL_LIBRARIES} ${LOG_LIBRARIES} ${SECURITY_LIBRARIES})
