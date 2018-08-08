#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#include <curl/curl.h>
#include <jansson.h>

#include "../waves-c/src/crypto/waves_crypto.h"
#include "../waves-c/src/crypto/base58/libbase58.h"
#include "../waves-c/src/crypto/transactions/transfer_transaction.h"
#include "../trezor-crypto/bip39.h"
#include "zap.h"

//
// -- Waves constants --
//

#define TESTNET_HOST "https://testnode1.wavesnodes.com"
#define MAINNET_HOST "https://nodes.wavesnodes.com"

#define TESTNET_ASSETID "CgUrFtinLXEbJwJVjwwcppk4Vpz1nMmR3H5cQaDcUcfe"
#define MAINNET_ASSETID "nada"

//
// -- Implementation details -- 
//

#define MAX_FILENAME 1024
#define MAX_URL 1024

#define MAX_CURL_DATA (1024*100)
struct curl_data_t
{
    int len;
    char ptr[MAX_CURL_DATA];
};

//
// -- Library globals --
//

char g_network = 'T';

//
// -- Logging definitions --
//

#define DEBUG 1
#ifdef __ANDROID__
    #include <android/log.h>
    #define debug_print(fmt, ...) \
        do { if (DEBUG) __android_log_print(ANDROID_LOG_ERROR, "LIBZAP", fmt, ##__VA_ARGS__); } while (0)
#else
    #define debug_print(fmt, ...) \
        do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)
#endif

//
// -- Internal functions --
//

void check_network()
{
    assert(g_network != 'T' || g_network != 'W');
}

const char* network_host()
{
    if (g_network == 'W')
        return MAINNET_HOST;
    return TESTNET_HOST;
}

const char* network_assetid()
{
    if (g_network == 'W')
        return MAINNET_ASSETID;
    return TESTNET_ASSETID;
}

#ifdef __ANDROID__
bool curl_cacert_pem_filename(char filename[MAX_FILENAME])
{
    char cmdline[MAX_FILENAME];
    memset(cmdline, 0, MAX_FILENAME);
    memset(filename, 0, MAX_FILENAME);
    FILE *f = fopen("/proc/self/cmdline", "r");
    if (f == NULL)
    {
        debug_print("unabled to open /proc/self/cmdline\n");
        return false;
    }
    size_t result = fread(cmdline, 1, MAX_FILENAME, f);
    fclose(f);
    if (result > 0)
    {
        int res = snprintf(filename, MAX_FILENAME, "/data/data/%s/cacert.pem", cmdline);
        if (res < 0 || res >= MAX_FILENAME)
        {
            debug_print("unable to create cacert.pem filename\n");
            return false;
        }
        return true;
    }
    debug_print("unable to read /proc/self/cmdline\n");
    return false;
}

bool cacert_pem_file_write()
{
#include "resources.c"
    // get the filename
    char filename[MAX_FILENAME];
    if (!curl_cacert_pem_filename(filename))
    {
        debug_print("unable to find cacert pem filename!");
        return false;
    }
    // create it if it does not exist
    if (access(filename, F_OK) == -1)
    {
        FILE *f = fopen(filename, "w");
        if (f == NULL)
            return false;
        size_t result = fwrite(cacert_pem, 1, sizeof(cacert_pem), f);
        fclose(f);
        if (result != sizeof(cacert_pem))
        {
            debug_print("failed to write %s (%d of %d bytes)", filename, result, sizeof(cacert_pem));
            return false;
        }
    }
    return true;
}
#endif

static size_t curl_write_data(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    struct curl_data_t* data = (struct curl_data_t*)userdata;
    int remaining = MAX_CURL_DATA - data->len;
    if (remaining < nmemb)
    {
        debug_print("curl_write_data: remaining (%d) <  nmemb (%zu)\n", remaining, nmemb);
        return 0;
    }
    memcpy(data->ptr + data->len, ptr, nmemb);
    data->len = data->len + nmemb;
    return nmemb;
}

bool get_url(const char *url, struct curl_data_t *data)
{
#ifdef __ANDROID__
    if (!cacert_pem_file_write())
    {
        debug_print("failed to write/check the cacert pem file\n");
        return false;
    }
#endif
    // reset data struct
    data->len = 0;
    memset(data->ptr, 0, MAX_CURL_DATA);
    // use curl to request
    CURL* curl = curl_easy_init();
    if (curl)
    {
        CURLcode res;
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);
#ifdef __ANDROID__
        char filename[MAX_FILENAME];
        if (curl_cacert_pem_filename(filename))
            curl_easy_setopt(curl, CURLOPT_CAINFO, filename);
#endif
        res = curl_easy_perform(curl);
        if (res == CURLE_OK)
        {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            if (response_code == 200)
            {
                curl_easy_cleanup(curl);
                return true;
            }
            debug_print("get_url: response_code (%ld, %s)\n", response_code, url);
        }
        debug_print("get_url: curl code (%u, %s)\n", res, url);
        curl_easy_cleanup(curl);
    }
    return false;
}

bool post_url(const char *url, const char *post_data)
{
#ifdef __ANDROID__
    if (!cacert_pem_file_write())
    {
        debug_print("failed to write/check the cacert pem file\n");
        return false;
    }
#endif
    // use curl to request
    CURL* curl = curl_easy_init();
    if (curl)
    {
        struct curl_slist *headers = NULL;
        CURLcode res;
        curl_easy_setopt(curl, CURLOPT_URL, url);
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
#ifdef __ANDROID__
        char filename[MAX_FILENAME];
        if (curl_cacert_pem_filename(filename))
            curl_easy_setopt(curl, CURLOPT_CAINFO, filename);
#endif
        res = curl_easy_perform(curl);
        if (res == CURLE_OK)
        {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            if (response_code == 200)
            {
                curl_easy_cleanup(curl);
                return true;
            }
            debug_print("post_url: response_code (%ld, %s)\n", response_code, url);
        }
        debug_print("post_url: curl code (%u, %s)\n", res, url);
        curl_easy_cleanup(curl);
    }
    return false;
}

bool get_waves_endpoint(const char *endpoint, struct curl_data_t *data)
{
    char url[MAX_URL];
    int res = snprintf(url, MAX_URL, "%s%s", network_host(), endpoint);
    if (res < 0 || res >= MAX_URL)
        return false;
    return get_url(url, data);
}

bool post_waves_endpoint(const char *endpoint, const char *post_data)
{
    char url[MAX_URL];
    int res = snprintf(url, MAX_URL, "%s%s", network_host(), endpoint);
    if (res < 0 || res >= MAX_URL)
        return false;
    return post_url(url, post_data);
}

void print_json_error(const char *function, json_error_t *error)
{
    debug_print("%s: %s - source: %s\n", function, error->text, error->source);
}

bool zap_securehash(void *digest, const void *data, size_t datasz)
{
    waves_secure_hash(data, datasz, digest);
    return true;
}

//
// -- PUBLIC FUNCTIONS --
//

int lzap_version()
{
    return 1;
}

void lzap_network_set(char network_byte)
{
    g_network = network_byte;
}

bool lzap_mnemonic_create(char *output, size_t size)
{
    const char *mnemonic = mnemonic_generate(128);
    if (mnemonic == NULL)
    {
        debug_print("mnemonic_generate failed\n");
        return false;
    }
    size_t len = strlen(mnemonic);
    if (len > size)
    {
        debug_print("output string not large enough (%zu bytes required)\n", len);
        return false;
    }
    strncpy(output, mnemonic, size); 
    return true;
}

bool lzap_mnemonic_check(const char *mnemonic)
{
    return mnemonic_check(mnemonic);
}

void lzap_seed_to_address(const char *seed, char *output)
{
    check_network();
    waves_seed_to_address(seed, g_network, output);
}

bool get_json_string(json_t *string_field, char *target, int max)
{
    if (!json_is_string(string_field))
    {
        if (json_is_null(string_field))
        {
            memset(target, 0, max);
            return true;
        }
        debug_print("string_field is not a string\n");
        return false;
    }
    const char *value = json_string_value(string_field);
    if (strlen(value) > max-1)
    {
        debug_print("string_field is too large (%zu > %d)\n", strlen(value), max-1);
        return false;
    }
    strncpy(target, value, max);
    return true;
}

bool get_json_int(json_t *int_field, long *target)
{
    if (!json_is_integer(int_field))
    {
        debug_print("int_field is not an int\n");
        return false;
    }
    *target = json_integer_value(int_field);
    return true;
}

bool get_json_string_from_object(json_t *object, const char *field_name, char *target, int max)
{
    json_t *field = json_object_get(object, field_name);
    return get_json_string(field, target, max);
}

bool get_json_int_from_object(json_t *object, const char *field_name, long *target)
{
    json_t *field = json_object_get(object, field_name);
    return get_json_int(field, target);
}

struct int_result_t lzap_address_balance(const char *address)
{
    check_network();

    struct int_result_t balance = { false, 0 };

    char endpoint[MAX_URL];
    int res = snprintf(endpoint, MAX_URL, "/assets/balance/%s/%s", address, network_assetid());
    if (res < 0 || res >= MAX_URL)
        return balance;
    struct curl_data_t data;
    if (get_waves_endpoint(endpoint, &data))
    {
        json_t *root;
        json_error_t error;
        root = json_loads(data.ptr, 0, &error);
        if (!root)
        {
            print_json_error("lzap_address_balance", &error);
            return balance;
        }
        if (!json_is_object(root))
        {
            debug_print("lzap_address_balance: json root is not an object\n");
            goto cleanup;
        }
        if (!get_json_int_from_object(root, "balance", &balance.value))
        {
            debug_print("lzap_address_balance: failed to get balance\n");
            goto cleanup;
        }
        balance.success = true;
cleanup:
        json_decref(root);
        return balance;
    }
    return balance;
}

struct int_result_t lzap_address_transactions(const char *address, struct tx_t *txs, int count)
{
    check_network();

    struct int_result_t result = { false, 0 };

    char endpoint[MAX_URL];
    int res = snprintf(endpoint, MAX_URL, "/transactions/address/%s/limit/%d", address, count);
    if (res < 0 || res >= MAX_URL)
        return result;
    struct curl_data_t data;
    if (get_waves_endpoint(endpoint, &data))
    {
        json_t *root;
        json_error_t error;
        root = json_loads(data.ptr, 0, &error);
        if (!root)
        {
            print_json_error("lzap_address_transactions", &error);
            return result;
        }
        if (!json_is_array(root))
        {
            debug_print("lzap_address_transactions: json root is not an array\n");
            goto cleanup;
        }
        if (!json_array_size(root) == 1)
        {
            debug_print("lzap_address_transactions: json root array is not of size 1\n");
            goto cleanup;
        }
        json_t *tx_array = json_array_get(root, 0);
        if (!json_is_array(tx_array))
        {
            debug_print("lzap_address_transactions: tx_array is not an array\n");
            goto cleanup;
        }
        for (int i = 0; i < json_array_size(tx_array); i++)
        {
            json_t *tx_object = json_array_get(tx_array, i);
            if (!json_is_object(tx_object))
            {
                debug_print("lzap_address_transactions: tx_object is not an object\n");
                goto cleanup;
            }
            if (!get_json_string_from_object(tx_object, "id", txs[i].id, MAX_TXFIELD))
            {
                debug_print("lzap_address_transactions: failed to get tx id\n");
                goto cleanup;
            }
            if (!get_json_string_from_object(tx_object, "sender", txs[i].sender, MAX_TXFIELD))
            {
                debug_print("lzap_address_transactions: failed to get tx sender\n");
                goto cleanup;
            }
            if (!get_json_string_from_object(tx_object, "recipient", txs[i].recipient, MAX_TXFIELD))
            {
                debug_print("lzap_address_transactions: failed to get tx recipient\n");
                goto cleanup;
            }
            if (!get_json_string_from_object(tx_object, "assetId", txs[i].asset_id, MAX_TXFIELD))
            {
                debug_print("lzap_address_transactions: failed to get tx asset_id\n");
                goto cleanup;
            }
            if (!get_json_string_from_object(tx_object, "feeAsset", txs[i].fee_asset, MAX_TXFIELD))
            {
                debug_print("lzap_address_transactions: failed to get tx fee_asset\n");
                goto cleanup;
            }
            if (!get_json_string_from_object(tx_object, "attachment", txs[i].attachment, MAX_TXFIELD))
            {
                debug_print("lzap_address_transactions: failed to get tx attachment\n");
                goto cleanup;
            }
            if (!get_json_int_from_object(tx_object, "amount", &txs[i].amount))
            {
                debug_print("lzap_address_transactions: failed to get tx amount\n");
                goto cleanup;
            }
            if (!get_json_int_from_object(tx_object, "fee", &txs[i].fee))
            {
                debug_print("lzap_address_transactions: failed to get tx fee\n");
                goto cleanup;
            }
            if (!get_json_int_from_object(tx_object, "timestamp", &txs[i].timestamp))
            {
                debug_print("lzap_address_transactions: failed to get tx timestamp\n");
                goto cleanup;
            }

            result.value = i + 1;
        }
        result.success = true;
cleanup:
        json_decref(root);
        return result;
    }
    return result;
}

struct int_result_t lzap_transaction_fee()
{
    check_network();

    struct int_result_t result = { false, 0 };

    // first get the asset info
    char endpoint[MAX_URL];
    int res = snprintf(endpoint, MAX_URL, "/assets/details/%s", network_assetid());
    if (res < 0 || res >= MAX_URL)
        return result;
    struct curl_data_t data;
    if (get_waves_endpoint(endpoint, &data))
    {
        json_t *root;
        json_error_t error;
        root = json_loads(data.ptr, 0, &error);
        if (!root)
        {
            print_json_error("lzap_transaction_fee: could not parse json", &error);
            return result;
        }
        if (!json_is_object(root))
        {
            debug_print("lzap_transaction_fee: json root is not an object\n");
            goto cleanup;
        }
        long min_asset_fee;
        if (!get_json_int_from_object(root, "minSponsoredAssetFee", &min_asset_fee))
        {
            debug_print("lzap_transaction_fee: failed to get min sponsored asset fee\n");
            goto cleanup;
        }
        long decimals;
        if (!get_json_int_from_object(root, "decimals", &decimals))
        {
            debug_print("lzap_transaction_fee: failed to get decimals\n");
            goto cleanup;
        }
        // sanity check
        assert(min_asset_fee / pow(10, decimals < 1));
        // return result
        result.value = min_asset_fee;
        result.success = true;
cleanup:
        json_decref(root);
        return result;
    }
    return result;

}

struct spend_tx_t lzap_transaction_create(const char *seed, const char *recipient, long amount, long fee, const char *attachment)
{
    debug_print("lzap_transaction_create: seed: %s, recipient: %s, amount: %ld, fee: %ld, attachment: %s\n", seed, recipient, amount, fee, attachment);

    check_network();

    struct spend_tx_t result = {};

    // get private and public key
    curve25519_secret_key privkey;
    curve25519_public_key pubkey;
    waves_seed_to_privkey(seed, privkey, pubkey); 

    // decode base58 recipient
    char recipient_bytes[26] = {};
    size_t recipient_bytes_sz = sizeof(recipient_bytes);
    if (!b58tobin(recipient_bytes, &recipient_bytes_sz, recipient, 0))
    {
        debug_print("lzap_transaction_create: failed to decode recipient\n");
        return result;
    }

    // check base58 recipient
    b58_securehash_impl = zap_securehash;
    int b58chk = b58check(recipient_bytes, recipient_bytes_sz, recipient);
    if (b58chk < 0)
    {
        debug_print("lzap_transaction_create: error checking base58 decoded recipient (%d)\n", b58chk);
        return result;
    }

    // decode base58 asset id
    char asset_id_bytes[32] = {};
    size_t asset_id_bytes_sz = sizeof(asset_id_bytes);
    if (!b58tobin(asset_id_bytes, &asset_id_bytes_sz, network_assetid(), 0))
    {
        debug_print("lzap_transaction_create: failed to decode asset id\n");
        return result;
    }

    // create structure
    TransferTransactionsBytes tx = {};
    tx.type = 4;
    memcpy(tx.sender_public_key, pubkey, sizeof(pubkey));
    tx.amount_asset_flag = 1;
    memset(tx.amount_asset_id, 0, sizeof(tx.amount_asset_id));
    memcpy(tx.amount_asset_id, asset_id_bytes, asset_id_bytes_sz);
    tx.fee_asset_flag = 1;
    memset(tx.fee_asset_id, 0, sizeof(tx.fee_asset_id));
    memcpy(tx.fee_asset_id, asset_id_bytes, asset_id_bytes_sz);
    tx.timestamp = time(NULL) * 1000;
    tx.amount = amount;
    tx.fee = fee;
    memset(tx.recipient_address_or_alias, 0, sizeof(tx.recipient_address_or_alias));
    memcpy(tx.recipient_address_or_alias, recipient_bytes, recipient_bytes_sz);
    memset(tx.attachment, 0, sizeof(tx.attachment));
    if (!attachment)
        tx.attachment_length = 0;
    else
    {
        tx.attachment_length = strlen(attachment);
        size_t sz = strlen(attachment);
        if (sz > sizeof(tx.attachment))
            sz = sizeof(tx.attachment);
        memcpy(tx.attachment, attachment, sz);
    }

    // convert to byte array
    if (!waves_transfer_transaction_to_bytes(&tx, result.tx_bytes, &result.tx_bytes_size, 0))
    {
        debug_print("lzap_transaction_create: failed to convert to bytes\n");
        return result;
    }

    // sign tx
    if (!waves_message_sign(&privkey, result.tx_bytes, result.tx_bytes_size, result.signature))
    {
        debug_print("lzap_transaction_create: failed to create signature\n");
        return result;
    }

    result.success = true;
    return result;
}

bool json_set_string(json_t *object, char *field, char *value)
{
    if (json_object_set_new(object, field, json_string(value)) == -1)
    {
        debug_print("json_set_string: failed to set json field '%s'\n", field);
        return false;
    }
    return true;
}

bool json_set_int(json_t *object, char *field, long value)
{
    if (json_object_set_new(object, field, json_integer(value)) == -1)
    {
        debug_print("json_set_int: failed to set json field '%s'\n", field);
        return false;
    }
    return true;
}

bool lzap_transaction_broadcast(struct spend_tx_t spend_tx)
{
    check_network();

    bool result = false;
    char *json_data = NULL;
    json_t *root = NULL;

    // first parse the transaction and encode the strings to base58
    TransferTransactionsBytes ttx_bytes;
    if (!waves_parse_transfer_transaction(spend_tx.tx_bytes, 0, &ttx_bytes))
    {
        debug_print("lzap_transaction_broadcast: failed to parse tx data\n");
        goto cleanup;
    }

    TransferTransactionsData ttx_data;
    if (!waves_read_transfer_transaction_data(&ttx_bytes, g_network, &ttx_data))
    {
        debug_print("lzap_transaction_broadcast: failed to convert tx bytes\n");
        goto cleanup;
    }
    char signature_b58[128];
    size_t tmp = sizeof(signature_b58);
    b58enc(signature_b58, &tmp, spend_tx.signature, sizeof(spend_tx.signature));

    // now create the json
    root = json_object();
    if (root == NULL)
    {
        debug_print("lzap_transaction_broadcast: failed to create root json object\n");
        goto cleanup;
    }
    if (!json_set_string(root, "assetId", ttx_data.amount_asset_id))
        goto cleanup;
    if (!json_set_string(root, "senderPublicKey", ttx_data.sender_public_key))
        goto cleanup;
    if (!json_set_string(root, "recipient", ttx_data.recipient_address_or_alias))
        goto cleanup;
    if (!json_set_int(root, "fee", ttx_data.fee))
        goto cleanup;
    if (!json_set_string(root, "feeAssetId", ttx_data.fee_asset_id))
        goto cleanup;
    if (!json_set_int(root, "amount", ttx_data.amount))
        goto cleanup;
    if (!json_set_string(root, "attachment", ttx_data.attachment))
        goto cleanup;
    if (!json_set_int(root, "timestamp", ttx_data.timestamp))
        goto cleanup;
    debug_print("%s\n", spend_tx.signature);
    if (!json_set_string(root, "signature", signature_b58))
        goto cleanup;
    json_data = json_dumps(root, 0);
    if (json_data == NULL)
    {
        debug_print("lzap_transaction_broadcast: failed to create json_data \n");
        goto cleanup;
    }
    debug_print("lzap_transaction_broadcast: json_data '%s'\n", json_data);

    // now broadcast the data
    char *endpoint = "/assets/broadcast/transfer";
    result = post_waves_endpoint(endpoint, json_data);

cleanup:
    if (root)
        json_decref(root);
    if (json_data)
        free(json_data);
    return result;
}

bool lzap_b58_enc(void *src, size_t src_sz, char *dst, size_t dst_sz)
{
    memset(dst, 0, dst_sz);
    return b58enc(dst, &dst_sz, src, src_sz);
}
