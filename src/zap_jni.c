#include <jni.h>
#include <string.h>
#include <stdlib.h>

#include "zap.h"

//
// -- Logging definitions --
//

#define DEBUG 1
#ifdef __ANDROID__
    #include <android/log.h>
    #define debug_print(fmt, ...) \
        do { if (DEBUG) __android_log_print(ANDROID_LOG_ERROR, "LIBZAPjni", fmt, ##__VA_ARGS__); } while (0)
#else
    #define debug_print(fmt, ...) \
        do { if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)
#endif

//
// -- Local helper functions --
//

jobject create_jni_int_result(JNIEnv *env, struct int_result_t ir)
{
    jclass cls = (*env)->FindClass(env, "com/djpsoft/zap/plugin/IntResult");
    jmethodID constructor = (*env)->GetMethodID(env, cls, "<init>", "(ZJ)V");
    return (*env)->NewObject(env, cls, constructor, ir.success, ir.value);
}

jobject create_jni_spend_tx(JNIEnv *env, struct spend_tx_t spendtx)
{
    jclass cls = (*env)->FindClass(env, "com/djpsoft/zap/plugin/SpendTx");
    jmethodID constructor = (*env)->GetMethodID(env, cls, "<init>", "(Z[B[B)V");
    jbyteArray data = (*env)->NewByteArray(env, spendtx.data_size);
    (*env)->SetByteArrayRegion(env, data, 0, spendtx.data_size, spendtx.data);
    jbyteArray signature = (*env)->NewByteArray(env, sizeof(spendtx.signature));
    (*env)->SetByteArrayRegion(env, signature, 0, sizeof(spendtx.signature), spendtx.signature);
    jobject obj = (*env)->NewObject(env, cls, constructor, spendtx.success, data, signature);
    return obj;
}

bool extract_jni_spend_tx(JNIEnv *env, jobject spend_tx, struct spend_tx_t *spend_tx_native)
{
    jclass cls = (*env)->FindClass(env, "com/djpsoft/zap/plugin/SpendTx");
    // get data
    jfieldID fieldid = (*env)->GetFieldID(env, cls, "Data", "[B");
    jbyteArray *data = NULL;
    if (fieldid == 0)
    {
        debug_print("failed to find data field :(");
        return false;
    }
    jobject arr = (*env)->GetObjectField(env, spend_tx, fieldid);
    data = (jbyteArray*)(&arr);
    jbyte *c_data = (*env)->GetByteArrayElements(env, *data, NULL);
    jsize data_sz = (*env)->GetArrayLength(env, *data);
    if (c_data)
    {
        // copy data
        if (data_sz > sizeof(spend_tx_native->data))
            data_sz = sizeof(spend_tx_native->data);
        memcpy(spend_tx_native->data, c_data, data_sz);  
        spend_tx_native->data_size = data_sz;
        (*env)->ReleaseByteArrayElements(env, *data, c_data, JNI_ABORT);
    }
    // get signature info
    fieldid = (*env)->GetFieldID(env, cls, "Signature", "[B");
    jbyteArray *signature = NULL;
    if (fieldid == 0)
    {
        debug_print("failed to find signature field :(");
        return false;
    }
    arr = (*env)->GetObjectField(env, spend_tx, fieldid);
    signature = (jbyteArray*)(&arr);
    jbyte *c_signature = (*env)->GetByteArrayElements(env, *signature, NULL);
    jsize signature_sz = (*env)->GetArrayLength(env, *signature);
    if (c_signature)
    {
        // copy data
        if (signature_sz > sizeof(spend_tx_native->signature))
            signature_sz = sizeof(spend_tx_native->signature);
        memcpy(spend_tx_native->signature, c_signature, signature_sz);  
        (*env)->ReleaseByteArrayElements(env, *signature, c_signature, JNI_ABORT);
    }
    return true;
}

bool set_jni_object_str(JNIEnv *env, jobject obj, char *name, char *val)
{
    jobject jni_value = (*env)->NewStringUTF(env, val);
    if (jni_value == NULL)
    {
        debug_print("error NewStringUTF result is %p", jni_value);
        return false;
    }
    jclass cls = (*env)->GetObjectClass(env, obj);
    if (!cls)
    {
        debug_print("error jclass is %p", cls);
        return false;
    }
    jfieldID fieldid = (*env)->GetFieldID(env, cls, name, "Ljava/lang/String;");
    if (fieldid == 0)
    {
        debug_print("error fieldid (for %s) is %d", name, (int)fieldid);
        return false;
    }
    (*env)->SetObjectField(env, obj, fieldid, jni_value);
    return true;
}

bool set_jni_object_long(JNIEnv *env, jobject obj, char *name, long long val)
{
    jclass cls = (*env)->GetObjectClass(env, obj);
    if (!cls)
    {
        debug_print("error jclass is %p", cls);
        return false;
    }
    jfieldID fieldid = (*env)->GetFieldID(env, cls, name, "J");
    if (fieldid == 0)
    {
        debug_print("error fieldid (for %s) is %d", name, (int)fieldid);
        return false;
    }
    (*env)->SetLongField(env, obj, fieldid, val);
    return true;
}

//
// -- Public functions --
//

JNIEXPORT jint JNICALL Java_com_djpsoft_zap_plugin_zap_1jni_version(JNIEnv* env, jobject thiz)
{
    return lzap_version();
}

JNIEXPORT jstring JNICALL Java_com_djpsoft_zap_plugin_zap_1jni_node_1get(
    JNIEnv* env, jobject thiz)
{
    const char *c_url = lzap_node_get();
    return (*env)->NewStringUTF(env, c_url);
}


JNIEXPORT void JNICALL Java_com_djpsoft_zap_plugin_zap_1jni_node_1set(
    JNIEnv* env, jobject thiz, jstring url)
{
    const char *c_url = (*env)->GetStringUTFChars(env, url, 0);
    lzap_node_set(c_url);
}

JNIEXPORT jchar JNICALL Java_com_djpsoft_zap_plugin_zap_1jni_network_1get(
    JNIEnv* env, jobject thiz)
{
    return lzap_network_get();
}


JNIEXPORT void JNICALL Java_com_djpsoft_zap_plugin_zap_1jni_network_1set(
    JNIEnv* env, jobject thiz, jchar network_byte)
{
    lzap_network_set(network_byte);
}

JNIEXPORT jobject JNICALL Java_com_djpsoft_zap_plugin_zap_1jni_mnemonic_1create(
    JNIEnv* env, jobject thiz)
{
    char output[1024];
    if (lzap_mnemonic_create(output, sizeof(output)))
        return (*env)->NewStringUTF(env, output);
    return (*env)->NewStringUTF(env, NULL);
}

JNIEXPORT jboolean JNICALL Java_com_djpsoft_zap_plugin_zap_1jni_mnemonic_1check(
    JNIEnv* env, jobject thiz, jstring mnemonic)
{
    const char *c_mnemonic = (*env)->GetStringUTFChars(env, mnemonic, 0);
    return lzap_mnemonic_check(c_mnemonic);
}

JNIEXPORT jobject JNICALL Java_com_djpsoft_zap_plugin_zap_1jni_mnemonic_1wordlist(
    JNIEnv* env, jobject thiz)
{
    const char* const* c_words = lzap_mnemonic_wordlist();
    const char* const* tmp = c_words;
    int size = 0;
    while (*tmp)
    {
        size++;
        tmp++;
    }
    jclass cls = (*env)->FindClass(env, "java/lang/String");
    jobjectArray words = (jobjectArray)(*env)->NewObjectArray(env, size, cls, NULL);
    for (int i = 0; i < size; i++)
    {
        jstring word = (*env)->NewStringUTF(env, c_words[i]);
        (*env)->SetObjectArrayElement(env, words, i, word);
    }
    return words;
}

JNIEXPORT jstring JNICALL Java_com_djpsoft_zap_plugin_zap_1jni_seed_1address(
    JNIEnv* env, jobject thiz, jstring seed)
{
    const char *c_seed = (*env)->GetStringUTFChars(env, seed, 0);
    char output[1024];
    lzap_seed_address(c_seed, output);
    return (*env)->NewStringUTF(env, output);
}

JNIEXPORT jobject JNICALL Java_com_djpsoft_zap_plugin_zap_1jni_address_1balance(
    JNIEnv* env, jobject thiz, jstring address)
{
    // create c compatible structures
    const char *c_address = (*env)->GetStringUTFChars(env, address, 0);
    // get result
    struct int_result_t balance = lzap_address_balance(c_address);
    // create java class to return result
    debug_print("address_balance: %s, %d, %lld", c_address, balance.success, balance.value);
    return create_jni_int_result(env, balance);
}

bool populate_jni_tx(JNIEnv* env, jobject jni_tx, struct tx_t *tx)
{
    if (!set_jni_object_str(env, jni_tx, "Id", tx->id))
        return false;
    if (!set_jni_object_str(env, jni_tx, "Sender", tx->sender))
        return false;
    if (!set_jni_object_str(env, jni_tx, "Recipient", tx->recipient))
        return false;
    if (!set_jni_object_str(env, jni_tx, "AssetId", tx->asset_id))
        return false;
    if (!set_jni_object_str(env, jni_tx, "FeeAsset", tx->fee_asset))
        return false;
    if (!set_jni_object_str(env, jni_tx, "Attachment", tx->attachment))
        return false;
    if (!set_jni_object_long(env, jni_tx, "Amount", tx->amount))
        return false;
    if (!set_jni_object_long(env, jni_tx, "Fee", tx->fee))
        return false;
    if (!set_jni_object_long(env, jni_tx, "Timestamp", tx->timestamp))
        return false;
    return true;
}

JNIEXPORT jobject JNICALL Java_com_djpsoft_zap_plugin_zap_1jni_address_1transactions(
    JNIEnv* env, jobject thiz, jstring address, jobjectArray txs, jint count)
{
    struct int_result_t result = {false, 0};
    // create c compatible structures
    const char *c_address = (*env)->GetStringUTFChars(env, address, 0);
    struct tx_t *c_txs = malloc(sizeof(struct tx_t) * count);
    if (c_txs)
    {
        // get result
        result = lzap_address_transactions(c_address, c_txs, count);
        if (result.success)
        {
            debug_print("got address transactions: %lld", result.value);
            // first we need to populate jni txs
            result.success = false;
            // populate jni txs
            for (int i = 0; i < result.value; i++)
            {
                debug_print("populate jni array element #%d", i);
                jobject tx = (*env)->GetObjectArrayElement(env, txs, i);
                if (!populate_jni_tx(env, tx, &c_txs[i]))
                    goto cleanup;
            }
            // all jni object properties set
            result.success = true;
        }
    }
cleanup:
    // free c_txs
    if (c_txs)
        free(c_txs);
    // create java class to return result
    return create_jni_int_result(env, result);
}

JNIEXPORT jobject JNICALL Java_com_djpsoft_zap_plugin_zap_1jni_transaction_1fee(
    JNIEnv* env, jobject thiz)
{
    // get result
    struct int_result_t fee = lzap_transaction_fee();
    // create java class to return result
    return create_jni_int_result(env, fee);
}

JNIEXPORT jobject JNICALL Java_com_djpsoft_zap_plugin_zap_1jni_transaction_1create(
    JNIEnv* env, jobject thiz, jstring seed, jstring recipient, jlong amount, jlong fee, jstring attachment)
{
    struct spend_tx_t spend_tx = {};
    // create c compatible structures
    if (!seed || !recipient)
    {
        debug_print("seed (%p) or recipient (%p) is null", seed, recipient);
        return create_jni_spend_tx(env, spend_tx);
    }
    const char *c_seed = (*env)->GetStringUTFChars(env, seed, 0);
    const char *c_recipient = (*env)->GetStringUTFChars(env, recipient, 0);
    const char *c_attachment = NULL;
    if (attachment)
        c_attachment = (*env)->GetStringUTFChars(env, attachment, 0);
    // get result
    spend_tx = lzap_transaction_create(c_seed, c_recipient, amount, fee, c_attachment);
    return create_jni_spend_tx(env, spend_tx);
}

JNIEXPORT jboolean JNICALL Java_com_djpsoft_zap_plugin_zap_1jni_transaction_1broadcast(
    JNIEnv* env, jobject thiz, jobject spend_tx, jobject broadcast_tx)
{
    // create c compatible structures
    struct spend_tx_t spend_tx_native = {};
    if (!extract_jni_spend_tx(env, spend_tx, &spend_tx_native))
        return false;
    // get result
    struct tx_t broadcast_tx_native;
    jboolean result = lzap_transaction_broadcast(spend_tx_native, &broadcast_tx_native);
    if (result)
        populate_jni_tx(env, broadcast_tx, &broadcast_tx_native);
    return result;
}

bool populate_jni_payment_request(JNIEnv* env, jobject jni_req, struct waves_payment_request_t *req)
{
    if (!set_jni_object_str(env, jni_req, "Address", req->address))
        return false;
    if (!set_jni_object_str(env, jni_req, "AssetId", req->asset_id))
        return false;
    if (!set_jni_object_str(env, jni_req, "Attachment", req->attachment))
        return false;
    if (!set_jni_object_long(env, jni_req, "Amount", req->amount))
        return false;
    return true;
}

JNIEXPORT jboolean JNICALL Java_com_djpsoft_zap_plugin_zap_1jni_uri_1parse(
    JNIEnv* env, jobject thiz, jstring uri, jobject payment_request)
{
    // create c compatible structures
    const char *c_uri = (*env)->GetStringUTFChars(env, uri, 0);
    struct waves_payment_request_t c_req = {};
    // get result
    jboolean result = lzap_uri_parse(c_uri, &c_req);
    if (result)
        populate_jni_payment_request(env, payment_request, &c_req);
    return result;
}
