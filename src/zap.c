#include "../waves-c/src/crypto/waves_crypto.h"

void lzap_seed_to_address(const unsigned char *key, unsigned char network_byte, unsigned char *output)
{
    waves_seed_to_address(key, network_byte, output);
}
