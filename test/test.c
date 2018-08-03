#include <stdio.h>
#include <stdbool.h>

#include "../src/zap.h"

int main()
{
    char* seed = "daniel";
    char address[1024];
    lzap_network_set('T');
    printf("libzap version %d\n", lzap_version());
    lzap_seed_to_address(seed, address);
    printf("%s - %s\n", seed, address);
    struct int_result_t balance = lzap_address_balance(address);
    printf("address balance success: %d\naddress balance value: %d\n", balance.success, balance.value);
    char mnemonic[1024];
    bool res = lzap_mnemonic_create(mnemonic, 1024);
    printf("mnemonic create: %d\nmnemonic: %s\n", res, mnemonic);
    printf("mnemonic check: %d\n", lzap_mnemonic_check(mnemonic));
    printf("mnemonic check bad: %d\n", !lzap_mnemonic_check("blah one two"));
}
