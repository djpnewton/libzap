#ifndef __ZAP_H__
#define __ZAP_H__

#include <stdbool.h>
#include <stdint.h>

#pragma pack(push, 1)
struct int_result_t
{
    uint32_t success;
    int64_t value;
};

#define MAX_TXFIELD 1024
struct tx_t
{
    uint64_t type;
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

#define MAX_TX_DATA_SIZE 364
#define SIG_SIZE 64
struct spend_tx_t
{
    uint32_t success;
    char data[MAX_TX_DATA_SIZE];
    uint32_t data_size;
    char signature[SIG_SIZE];
};

struct waves_payment_request_t
{
    char address[MAX_TXFIELD];
    char asset_id[MAX_TXFIELD];
    char attachment[MAX_TXFIELD];
    uint64_t amount;
};
#pragma pack(pop)

#define LZAP_ERR_NONE 0
#define LZAP_ERR_INVALID_NETWORK 1
#define LZAP_ERR_NETWORK_UNREACHABLE 2
#define LZAP_ERR_INVALID_ADDRESS 3
#define LZAP_ERR_INVALID_ATTACHMENT 4
#define LZAP_ERR_UNAVAILABLE_FUNDS 5
#define LZAP_ERR_INSUFFICIENT_FEE 6
#define LZAP_ERR_INVALID_WAVES_URI 7
#define LZAP_ERR_INVALID_ASSET_ID 8
#define LZAP_ERR_UNSPECIFIED -1

#define LZAP_VERSION 10

#ifdef __cplusplus
extern "C" {
#endif
    void lzap_error(int *code, const char** msg);
    int lzap_version();
    const char* lzap_node_get();
    void lzap_node_set(const char *url);
    char lzap_network_get();
    bool lzap_network_set(char network_byte);
    bool lzap_mnemonic_create(char *output, size_t size);
    bool lzap_mnemonic_check(const char *mnemonic);
    const char* const* lzap_mnemonic_wordlist();
    void lzap_seed_address(const char *seed, char *output);
    struct int_result_t lzap_address_check(const char *address);
    struct int_result_t lzap_address_balance(const char *address);
    struct int_result_t lzap_address_transactions(const char *address, struct tx_t *txs, int count);
    struct int_result_t lzap_address_transactions2(const char *address, struct tx_t *txs, int count, const char *after);
    struct int_result_t lzap_transaction_fee();
    struct spend_tx_t lzap_transaction_create(const char *seed, const char *recipient, uint64_t amount, uint64_t fee, const char *attachment);
    bool lzap_transaction_broadcast(struct spend_tx_t spend_tx, struct tx_t *broadcast_tx_out);
    bool lzap_uri_parse(const char *uri, struct waves_payment_request_t *req_out);

    bool lzap_b58_enc(void *src, size_t src_sz, char *dst, size_t dst_sz);

    // temporary wrappers while we wait for dart to be able to handle passing structs on the stack :))
    bool lzap_address_check_ns(const char *address);
    bool lzap_address_balance_ns(const char *address, int64_t *balance_out);
    bool lzap_address_transactions2_ns(const char *address, struct tx_t *txs, int count, const char *after, int64_t *count_out);
    bool lzap_transaction_fee_ns(int64_t *fee_out);
    void lzap_transaction_create_ns(const char *seed, const char *recipient, uint64_t amount, uint64_t fee, const char *attachment, struct spend_tx_t *spend_tx_out);
    bool lzap_transaction_broadcast_ns(struct spend_tx_t *spend_tx, struct tx_t *broadcast_tx_out);
#ifdef __cplusplus
}
#endif

#endif
