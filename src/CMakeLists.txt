cmake_minimum_required(VERSION 3.0)

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

add_subdirectory(../waves-c build/waves-c)
add_subdirectory(../deps/jansson-2.11 build/jansson)

project(zap)

set(TREZOR ../trezor-crypto/)
set(BIP39_SOURCE ${TREZOR}/bip39.c ${TREZOR}/hmac.c ${TREZOR}/rand.c ${TREZOR}/sha2.c ${TREZOR}/pbkdf2.c ${TREZOR}/memzero.c)

set(SOURCE zap.c)

if (ANDROID)
    create_resources(resources resources.c)
    set(SOURCE ${SOURCE} zap_jni.c)
    include_directories(${CURL_INCLUDE_DIR})
    include_directories(../deps/jansson-2.11/src)
    include_directories(../deps/jansson-2.11/android)
    set(ZLIB_LIBRARY "-lz")
    find_package(ZLIB REQUIRED) 
    set(LOG_LIBRARIES log)
endif()

set(CURL_LIBRARY "-lcurl") 
find_package(CURL REQUIRED) 
add_library(${PROJECT_NAME} SHARED ${SOURCE} ${BIP39_SOURCE})
target_link_libraries(${PROJECT_NAME} waves_c jansson ${CURL_LIBRARIES} ${ZLIB_LIBRARIES} ${LOG_LIBRARIES})
