#ifndef __WAVES_CRYPTO_H__
#define __WAVES_CRYPTO_H__

int lzap_version();
void lzap_seed_to_address(const unsigned char *key, unsigned char network_byte, unsigned char *output);

#endif
