#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <curl/curl.h>
#include <jansson.h>

#include "../waves-c/src/crypto/waves_crypto.h"
#include "../trezor-crypto/bip39.h"
#include "zap.h"

//
// -- Waves constants --
//

#define TESTNET_HOST "https://testnode1.wavesnodes.com"
#define MAINNET_HOST "https://nodes.wavesnodes.com"

#define TESTNET_ASSETID "35twb3NRL7cZwD1JjPHGzFLQ1P4gtUutTuFEXAg1f1hG"
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
    assert(g_network != 'T' || g_network != 'M');
}

const char* network_host()
{
    if (g_network == 'M')
        return MAINNET_HOST;
    return TESTNET_HOST;
}

const char* network_assetid()
{
    if (g_network == 'M')
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
        curl_easy_cleanup(curl);
        if (res == CURLE_OK)
        {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            if (response_code == 200)
                return true;
            debug_print("get_url: response_code (%ld, %s)\n", response_code, url);
        }
        debug_print("get_url: curl code (%u, %s)\n", res, url);
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

void print_json_error(const char *function, json_error_t *error)
{
    debug_print("%s: %s - source: %s\n", function, error->text, error->source);
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

bool lzap_mnemonic_check(char *mnemonic)
{
    return mnemonic_check(mnemonic);
}

void lzap_seed_to_address(const char *key, char *output)
{
    check_network();
    waves_seed_to_address(key, g_network, output);
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
        json_t* balance_field = json_object_get(root, "balance");
        if (!json_is_integer(balance_field))
        {
            debug_print("lzap_address_balance: balance field is not an integer\n");
            goto cleanup;
        }
        balance.value = json_integer_value(balance_field);
        balance.success = true;
cleanup:
        json_decref(root);
        return balance;
    }
    return balance;
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
            printf("tx#%d\n", i);
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
