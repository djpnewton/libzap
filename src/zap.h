#ifndef __ZAP_H__
#define __ZAP_H__

#ifdef __cplusplus
extern "C" {
#endif
    int lzap_version();
    void lzap_set_network(unsigned char network_byte);
    void lzap_seed_to_address(const unsigned char *key, unsigned char *output);
    bool lzap_address_balance(const unsigned char *address, int *balance);
    int lzap_test_curl();
    int lzap_test_jansson();
#ifdef __cplusplus
}
#endif

#endif
