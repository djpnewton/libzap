#ifndef __ZAP_H__
#define __ZAP_H__

#include <stdbool.h>

struct int_result_t
{
    bool success;
    int value;
};


#ifdef __cplusplus
extern "C" {
#endif
    int lzap_version();
    void lzap_network_set(char network_byte);
    bool lzap_mnemonic_create(char *output, size_t size);
    bool lzap_mnemonic_check(char *mnemonic);
    void lzap_seed_to_address(const char *key, char *output);
    struct int_result_t lzap_address_balance(const char *address);
#ifdef __cplusplus
}
#endif

#endif
