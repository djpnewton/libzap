#include <string.h>
#include <curl/curl.h>
#include <jansson.h>
#include "../waves-c/src/crypto/waves_crypto.h"

int lzap_version()
{
    return 1;
}

void lzap_seed_to_address(const unsigned char *key, unsigned char network_byte, unsigned char *output)
{
    waves_seed_to_address(key, network_byte, output);
}

bool lzap_test_curl()
{
    CURL *curl = curl_easy_init();
    if (curl)
    {
        CURLcode res;
        curl_easy_setopt(curl, CURLOPT_URL, "http://example.com");
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        if (res == CURLE_OK)
            return true;
    }
    return false;
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
