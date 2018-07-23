#include "stdio.h"

#include "../waves-c/src/crypto/waves_crypto.h"

int main()
{
    char* seed = "daniel";
    char address[1024];
    waves_seed_to_address(seed, 'T', address);
    printf("%s - %s\n", seed, address);
}
