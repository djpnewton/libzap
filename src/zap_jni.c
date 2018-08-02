#include <jni.h>
#include <string.h>

#include "zap.h"

JNIEXPORT jint JNICALL Java_com_djpsoft_zap_plugin_zap_1jni_version(JNIEnv* env, jobject thiz)
{
    return lzap_version();
}

JNIEXPORT void JNICALL Java_com_djpsoft_zap_plugin_zap_1jni_set_1network(
    JNIEnv* env, jobject thiz, jchar network_byte)
{
    lzap_set_network(network_byte);
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
    // get result
    char* c_address = strdup((*env)->GetStringUTFChars(env, address, 0));
    struct int_result_t balance = lzap_address_balance(c_address);
    // create java class to return result
    jclass cls = (*env)->FindClass(env, "com/djpsoft/zap/plugin/IntResult");
    jmethodID constructor = (*env)->GetMethodID(env, cls, "<init>", "(ZI)V");
    jobject instance = (*env)->NewObject(env, cls, constructor, balance.success, balance.value);
    return instance;
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


JNIEXPORT jboolean JNICALL Java_com_djpsoft_zap_plugin_zap_1jni_test_1curl(JNIEnv* env, jobject thiz)
{
    return lzap_test_curl();
}

JNIEXPORT jboolean JNICALL Java_com_djpsoft_zap_plugin_zap_1jni_test_1jansson(JNIEnv* env, jobject thiz)
{
    return lzap_test_jansson();
}
