#include "stdio.h"

#include "../src/zap.h"

int main()
{
    char* seed = "daniel";
    char address[1024];
    printf("libzap version %d\n", lzap_version());
    lzap_seed_to_address(seed, 'T', address);
    printf("%s - %s\n", seed, address);
    printf("test curl: %d\n", lzap_test_curl());
    printf("test jansson: %d\n", lzap_test_jansson());
}
