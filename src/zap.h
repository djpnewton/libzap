#ifndef __ZAP_H__
#define __ZAP_H__

#include <stdbool.h>
#include <stdint.h>

struct int_result_t
{
    bool success;
    int64_t value;
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
    uint64_t amount;
    uint64_t fee;
    uint64_t timestamp;
};

#define MAX_TX_BYTES 364
#define SIG_SIZE 64
struct spend_tx_t
{
    bool success;
    char tx_data[MAX_TX_BYTES];
    size_t tx_data_size;
    char signature[SIG_SIZE];
};

#ifdef __cplusplus
extern "C" {
#endif
    int lzap_version();
    void lzap_network_set(char network_byte);
    bool lzap_mnemonic_create(char *output, size_t size);
    bool lzap_mnemonic_check(const char *mnemonic);
    void lzap_seed_to_address(const char *seed, char *output);
    struct int_result_t lzap_address_balance(const char *address);
    struct int_result_t lzap_address_transactions(const char *address, struct tx_t *txs, int count);
    struct int_result_t lzap_transaction_fee();
    struct spend_tx_t lzap_transaction_create(const char *seed, const char *recipient, uint64_t amount, uint64_t fee, const char *attachment);
    bool lzap_transaction_broadcast(struct spend_tx_t spend_tx);

    bool lzap_b58_enc(void *src, size_t src_sz, char *dst, size_t dst_sz);
#ifdef __cplusplus
}
#endif

#endif
