#include <assert.h>
#include <stdio.h>
#include <stdbool.h>

#include "../src/zap.h"

void print_hex(unsigned char *buf, size_t sz)
{
    for (int i = 0; i < sz; i++)
        printf("%02x", buf[i]);
}

int main()
{
    // version
    printf("libzap version %d\n", lzap_version());

    // network set
    lzap_network_set('T');

    // mnemonic
    char mnemonic[1024];
    bool res = lzap_mnemonic_create(mnemonic, 1024);
    printf("mnemonic create: %d\nmnemonic: %s\n", res, mnemonic);
    printf("mnemonic check: %d\n", lzap_mnemonic_check(mnemonic));
    printf("mnemonic check bad: %d\n", !lzap_mnemonic_check("blah one two"));

    // seed to address
    char* seed = "daniel";
    char address[1024];
    lzap_seed_to_address(seed, address);
    printf("%s - %s\n", seed, address);

    // address balance
    struct int_result_t balance = lzap_address_balance(address);
    printf("address balance success: %d\naddress balance value: %ld\n", balance.success, balance.value);

    // address transactions 
    struct tx_t txs[100];
    struct int_result_t result = lzap_address_transactions(address, txs, 100);
    printf("address transactions success: %d\n", result.success);
    if (result.success)
        for (int i = 0; i < result.value; i++)
        {
            printf("  tx-%d\n    id: %s\n    sender: %s\n    recipient: %s\n",
                i, txs[i].id, txs[i].sender, txs[i].recipient);
            printf("    asset id: %s\n    fee asset: %s\n    attachment: %s\n",
                txs[i].asset_id, txs[i].fee_asset, txs[i].attachment);
            printf("    amount: %ld\n    fee: %ld\n    timestamp: %ld\n",
                txs[i].amount, txs[i].fee, txs[i].timestamp);
        }

    // spend tx
    struct int_result_t fee = lzap_transaction_fee();
    assert(fee.success);
    printf("transaction fee: %lu\n", fee.value);
    struct spend_tx_t tx = lzap_transaction_create(mnemonic, address, 100, fee.value, "blah blah");
    printf("transaction create:\n\tsuccess: %d\n\tbytes: %s\n\tlength: %lu\n\tsignature: ", tx.success, tx.tx_bytes, tx.tx_bytes_size);
    print_hex(tx.signature, sizeof(tx.signature));
    printf("\n");

    // broadcast tx
    printf("transaction broadcast: %d\n", lzap_transaction_broadcast(tx));
}
