#include <stdio.h>
#include <stdbool.h>

#include "../src/zap.h"

int main()
{
    char* seed = "daniel";
    char address[1024];
    lzap_set_network('T');
    printf("libzap version %d\n", lzap_version());
    lzap_seed_to_address(seed, address);
    printf("%s - %s\n", seed, address);
    struct int_result_t balance = lzap_address_balance(address);
    printf("address balance success: %d\naddress balance value: %d\n", balance.success, balance.value);
    printf("test curl: %d\n", lzap_test_curl());
    printf("test jansson: %d\n", lzap_test_jansson());
}
