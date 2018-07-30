#include <assert.h>
#include <string.h>

#include <curl/curl.h>
#include <jansson.h>

#include "../waves-c/src/crypto/waves_crypto.h"

#define TESTNET_HOST "https://testnode1.wavesnodes.com"
#define MAINNET_HOST "https://nodes.wavesnodes.com"

#define MAX_CURL_DATA (1024*20)
struct curl_data_t
{
    int len;
    char ptr[MAX_CURL_DATA];
};

unsigned char g_network = 'T';

#define DEBUG 1
#define debug_print(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

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
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        if (res == CURLE_OK)
        {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            if (response_code == 200)
                return true;
            debug_print("get_url: response_code (%ld)\n", response_code);
        }
        debug_print("get_url: curl code (%u)\n", res);
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

void lzap_set_network(unsigned char network_byte)
{
    g_network = network_byte;
}

int lzap_version()
{
    return 1;
}

void lzap_seed_to_address(const unsigned char *key, unsigned char *output)
{
    check_network();
    waves_seed_to_address(key, g_network, output);
}

int lzap_address_balance(const unsigned char *address)
{
    check_network();
    struct curl_data_t data;
    if (get_waves_endpoint("/assets/balance/{address}", &data))
    {
        //TODO
    }
    return false;
}

bool lzap_test_curl()
{
    struct curl_data_t data;
    bool res = get_url("http://example.com", &data);
    if (res)
    {
        printf("got url:\n");
        printf("%s\n", data.ptr);
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
