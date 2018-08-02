#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <curl/curl.h>
#include <jansson.h>

#include "../waves-c/src/crypto/waves_crypto.h"
#include "../trezor-crypto/bip39.h"
#include "zap.h"

#define TESTNET_HOST "https://testnode1.wavesnodes.com"
#define MAINNET_HOST "https://nodes.wavesnodes.com"

#define TESTNET_ASSETID "35twb3NRL7cZwD1JjPHGzFLQ1P4gtUutTuFEXAg1f1hG"
#define MAINNET_ASSETID "nada"

#define MAX_FILENAME 1024

#define MAX_CURL_DATA (1024*20)
struct curl_data_t
{
    int len;
    char ptr[MAX_CURL_DATA];
};

unsigned char g_network = 'T';

#define DEBUG 1
#ifdef __ANDROID__
    #include <android/log.h>
    #define debug_print(fmt, ...) \
        do { if (DEBUG) __android_log_print(ANDROID_LOG_ERROR, "LIBZAP", fmt, ##__VA_ARGS__); } while (0)
#else
    #define debug_print(fmt, ...) \
        do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)
#endif

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
        return false;
    size_t result = fread(cmdline, 1, MAX_FILENAME, f);
    fclose(f);
    if (result > 0)
    {
        debug_print("command line: %s\n", cmdline);
        int res = snprintf(filename, MAX_FILENAME, "/data/data/%s/cacert.pem", cmdline);
        if (res < 0 || res >= MAX_FILENAME)
            return false;
        debug_print("cacert_pem_filename: %s\n", filename);
        return true;
    }
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

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    struct curl_data_t* data = (struct curl_data_t*)userdata;
    int remaining = MAX_CURL_DATA - data->len;
    if (remaining < nmemb)
    {
        debug_print("write_data: remaining (%d) <  nmemb (%lu)\n", remaining, nmemb);
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
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
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
#define MAX_URL 1024
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

int lzap_version()
{
    return 1;
}

void lzap_set_network(unsigned char network_byte)
{
    g_network = network_byte;
}

void lzap_seed_to_address(const unsigned char *key, unsigned char *output)
{
    check_network();
    waves_seed_to_address(key, g_network, output);
}

struct int_result_t lzap_address_balance(const unsigned char *address)
{
    check_network();

    struct int_result_t balance = { false, 0 };

#define MAX_URL 1024
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

bool lzap_mnemonic_create(char *output, size_t size)
{
    const char *mnemonic = mnemonic_generate(128);
    if (mnemonic == NULL)
        return false;
    size_t len = strlen(mnemonic);
    if (len > size)
    {
        debug_print("output string not large enough (%lu bytes required)\n", len);
        return false;
    }
    strncpy(output, mnemonic, size); 
    return true;
}

bool lzap_test_curl()
{
    struct curl_data_t data;
    bool res = get_url("http://example.com", &data);
    if (res)
    {
        //printf("got url:\n");
        //printf("%s\n", data.ptr);
    }
    return res;
}

bool lzap_test_jansson()
{
    bool result = false;
    json_t *root;
    json_error_t error;
    root = json_loads("{\"test\": 123}", 0, &error);
    if (!root)
        return result;
    if (!json_is_object(root))
        goto cleanup;
    json_t* test = json_object_get(root, "test");
    if (!json_is_integer(test))
        goto cleanup;
    result = json_integer_value(test) == 123;
cleanup:
    json_decref(root);
    return result;
}
