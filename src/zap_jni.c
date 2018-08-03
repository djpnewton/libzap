#include <jni.h>
#include <string.h>

#include "zap.h"

// -- Local helper functions --

jobject create_jni_int_result(struct int_result_t ir)
{
    jclass cls = (*env)->FindClass(env, "com/djpsoft/zap/plugin/IntResult");
    jmethodID constructor = (*env)->GetMethodID(env, cls, "<init>", "(ZI)V");
    jobject instance = (*env)->NewObject(env, cls, constructor, ir.success, ir.value);
    return instance;
}

bool set_jni_object_str(jobject obj, char *name, char *val)
{
    jobject jni_value = (*env)->NewStringUTF(env, val);
    if (value == NULL)
       return false;
    jclass cls = (*env)->GetObjectClass(env, obj);
    jfieldID fieldid = (*env)->GetFieldID(env, cls, name, "Ljava/lang/String;");
    (*env)->SetObjectField(env, obj, fieldid, jni_value);
    return true;
}

bool set_jni_object_int(jobject obj, int val)
{
    jclass cls = (*env)->GetObjectClass(env, obj);
    jfieldID fieldid = (*env)->GetFieldID(env, cls, name, "I");
    (*env)->SetIntField(env, obj, fieldid, val);
    return true;
}

// -- Public functions --

JNIEXPORT jint JNICALL Java_com_djpsoft_zap_plugin_zap_1jni_version(JNIEnv* env, jobject thiz)
{
    return lzap_version();
}

JNIEXPORT void JNICALL Java_com_djpsoft_zap_plugin_zap_1jni_set_1network(
    JNIEnv* env, jobject thiz, jchar network_byte)
{
    lzap_network_set(network_byte);
}

JNIEXPORT void JNICALL Java_com_djpsoft_zap_plugin_zap_1jni_mnemonic_1create(
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
    char* c_mnemonic = strdup((*env)->GetStringUTFChars(env, mnemonic, 0));
    return lzap_mnemonic_check(c_mnemonic);
}

JNIEXPORT jstring JNICALL Java_com_djpsoft_zap_plugin_zap_1jni_seed_1to_1address(
    JNIEnv* env, jobject thiz, jstring key)
{
    char* c_key = strdup((*env)->GetStringUTFChars(env, key, 0));
    char output[1024];
    lzap_seed_to_address(c_key, output);
    return (*env)->NewStringUTF(env, output);
}

JNIEXPORT jobject JNICALL Java_com_djpsoft_zap_plugin_zap_1jni_address_1balance(
    JNIEnv* env, jobject thiz, jstring address)
{
    // create c compatible structures
    char* c_address = strdup((*env)->GetStringUTFChars(env, address, 0));
    // get result
    struct int_result_t balance = lzap_address_balance(c_address);
    // create java class to return result
    return return create_jni_int_result(balance);
}

JNIEXPORT jobject JNICALL Java_com_djpsoft_zap_plugin_zap_1jni_address_1transactions(
    JNIEnv* env, jobject thiz, jstring address, jobjectArray txs, jint count)
{
    struct int_result_t result = {false, 0};
    // create c compatible structures
    char* c_address = strdup((*env)->GetStringUTFChars(env, address, 0));
    struct tx_t *c_txs = malloc(sizeof(struct tx_t) * count);
    if (c_txs)
    {
        // get result
        result = lzap_address_transactions(c_address, c_txs, count);
        if (result.success)
        {
            // first we need to populate jni txs
            result.success = false;
            // populate jni txs
            for (int i = 0; i < result.value; i++)
            {
                jobject tx = (*env)->GetObjectArrayElement(txs, i);
                if (!set_jni_object_str(tx, "Id", c_txs[i].id))
                    goto cleanup;
                if (!set_jni_object_str(tx, "Sender", c_txs[i].sender))
                    goto cleanup;
                if (!set_jni_object_str(tx, "Recipient", c_txs[i].recipient))
                    goto cleanup;
                if (!set_jni_object_str(tx, "AssetId", c_txs[i].asset_id))
                    goto cleanup;
                if (!set_jni_object_str(tx, "FeeAsset", c_txs[i].fee_asset))
                    goto cleanup;
                if (!set_jni_object_str(tx, "Attachment", c_txs[i].attachment))
                    goto cleanup;
                if (!set_jni_object_int(tx, "Amount", c_txs[i].amount))
                    goto cleanup;
                if (!set_jni_object_int(tx, "Fee", c_txs[i].fee))
                    goto cleanup;
                if (!set_jni_object_int(tx, "Timestamp", c_txs[i].timestamp))
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
    return return create_jni_int_result(result);
}
