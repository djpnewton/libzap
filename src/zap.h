#ifndef __ZAP_H__
#define __ZAP_H__

#include <stdbool.h>

struct int_result_t
{
    bool success;
    int value;
};

#define MAX_TXFIELD 1024
struct tx_t
{
    char id[MAX_TXFIELD];
    char sender[MAX_TXFIELD];
    char recipient[MAX_TXFIELD];
    char asset_id[MAX_TXFIELD];
    char fee_asset[MAX_TXFIELD];
    char attachment[MAX_TXFIELD];
    long amount;
    long fee;
    long timestamp;
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
    struct int_result_t lzap_address_transactions(const char *address, struct tx_t *txs, int count);
#ifdef __cplusplus
}
#endif

#endif
