#include <jni.h>
#include <string.h>

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
    jbyteArray txdata = (*env)->NewByteArray(env, spendtx.tx_bytes_size);
    (*env)->SetByteArrayRegion(env, txdata, 0, spendtx.tx_bytes_size, spendtx.tx_bytes);
    jbyteArray signature = (*env)->NewByteArray(env, sizeof(spendtx.signature));
    (*env)->SetByteArrayRegion(env, signature, 0, sizeof(spendtx.signature), spendtx.signature);
    return (*env)->NewObject(env, cls, constructor, spendtx.success, txdata, signature);
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

bool set_jni_object_long(JNIEnv *env, jobject obj, char *name, int val)
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

JNIEXPORT jstring JNICALL Java_com_djpsoft_zap_plugin_zap_1jni_seed_1to_1address(
    JNIEnv* env, jobject thiz, jstring seed)
{
    const char *c_seed = (*env)->GetStringUTFChars(env, seed, 0);
    char output[1024];
    lzap_seed_to_address(c_seed, output);
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
    debug_print("address_balance: %s, %d, %ld", c_address, balance.success, balance.value);
    return create_jni_int_result(env, balance);
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
            debug_print("got address transactions: %ld", result.value);
            // first we need to populate jni txs
            result.success = false;
            // populate jni txs
            for (int i = 0; i < result.value; i++)
            {
                debug_print("populate jni array element #%d", i);
                jobject tx = (*env)->GetObjectArrayElement(env, txs, i);
                if (!set_jni_object_str(env, tx, "Id", c_txs[i].id))
                    goto cleanup;
                if (!set_jni_object_str(env, tx, "Sender", c_txs[i].sender))
                    goto cleanup;
                if (!set_jni_object_str(env, tx, "Recipient", c_txs[i].recipient))
                    goto cleanup;
                if (!set_jni_object_str(env, tx, "AssetId", c_txs[i].asset_id))
                    goto cleanup;
                if (!set_jni_object_str(env, tx, "FeeAsset", c_txs[i].fee_asset))
                    goto cleanup;
                if (!set_jni_object_str(env, tx, "Attachment", c_txs[i].attachment))
                    goto cleanup;
                if (!set_jni_object_long(env, tx, "Amount", c_txs[i].amount))
                    goto cleanup;
                if (!set_jni_object_long(env, tx, "Fee", c_txs[i].fee))
                    goto cleanup;
                if (!set_jni_object_long(env, tx, "Timestamp", c_txs[i].timestamp))
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
    debug_print("returning address transactions: %d, %ld", result.success, result.value);
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
    JNIEnv* env, jobject thiz, jstring seed, jstring recipient, jint amount, jstring attachment)
{
    struct spend_tx_t spend_tx = {};
    // get fee
    struct int_result_t fee = lzap_transaction_fee();
    if (!fee.success)
    {
        debug_print("unable to get fee");
        return create_jni_spend_tx(env, spend_tx);
    }
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
    spend_tx = lzap_transaction_create(c_seed, c_recipient, amount, fee.value, c_attachment);
    return create_jni_spend_tx(env, spend_tx);
}

JNIEXPORT jboolean JNICALL Java_com_djpsoft_zap_plugin_zap_1jni_transaction_1broadcast(
    JNIEnv* env, jobject thiz, jbyteArray txdata, jbyteArray signature)
{
    // create c compatible structures
    jbyte *c_txdata = (*env)->GetByteArrayElements(env, txdata, NULL);
    jsize txdata_sz = (*env)->GetArrayLength(env, txdata);
    jbyte *c_signature = (*env)->GetByteArrayElements(env, signature, NULL);
    jsize signature_sz = (*env)->GetArrayLength(env, txdata);
    struct spend_tx_t spend_tx = {};

    if (txdata_sz > sizeof(spend_tx.tx_bytes))
        txdata_sz = sizeof(spend_tx.tx_bytes);
    memcpy(spend_tx.tx_bytes, c_txdata, txdata_sz);  
    spend_tx.tx_bytes_size = txdata_sz;
    if (signature_sz > sizeof(spend_tx.signature))
        signature_sz = sizeof(spend_tx.signature);
    memcpy(spend_tx.signature, c_signature, signature_sz);  
    // get result
    return lzap_transaction_broadcast(spend_tx);
}

