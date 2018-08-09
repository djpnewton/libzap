project( waves-c )

file(GLOB CS "crypto/*.c" "crypto/base58/*.c"
        "crypto/blake2b/ref/*.c" "crypto/transactions/*.c" "crypto/libcurve25519-donna/*.c" "crypto/libcurve25519-donna/additions/*.c"
        "crypto/libcurve25519-donna/nacl_includes/*.c"  "crypto/libcurve25519-donna/nacl_sha512/*.c")

add_library(waves_c STATIC ${CS} crypto/utils.h crypto/utils.c crypto/utils.h)
target_link_libraries(waves_c ${OPENSSL_LIBRARIES} m)
add_definitions(-Wall -O3 -Wextra -Wno-format -fPIC)

add_executable( waves_vanity vanity.c )

if (ANDROID)
    set(PTHREAD_LIB)
else()
    set(PTHREAD_LIB pthread)
endif()

target_link_libraries(waves_vanity ${PTHREAD_LIB} waves_c)
install(TARGETS waves_vanity DESTINATION bin)
