#ifndef __ZAP_H__
#define __ZAP_H__

#ifdef __cplusplus
extern "C" {
#endif
    int lzap_version();
    void lzap_seed_to_address(const unsigned char *key, unsigned char network_byte, unsigned char *output);
#ifdef __cplusplus
}
#endif

#endif
