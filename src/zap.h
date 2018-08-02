#ifndef __ZAP_H__
#define __ZAP_H__

#include <stdbool.h>

#define MAX_ERR 1024
struct int_result_t
{
    bool success;
    int value;
};


#ifdef __cplusplus
extern "C" {
#endif
    int lzap_version();
    void lzap_set_network(unsigned char network_byte);
    void lzap_seed_to_address(const unsigned char *key, unsigned char *output);
    struct int_result_t lzap_address_balance(const unsigned char *address);
    bool lzap_mnemonic_create(char *output, size_t size);


    bool lzap_test_curl();
    bool lzap_test_jansson();
#ifdef __cplusplus
}
#endif

#endif
