#include <jni.h>
#include <string.h>

#include "zap.h"

JNIEXPORT jint JNICALL Java_com_djpsoft_zap_plugin_zap_1jni_version(JNIEnv* env, jobject thiz)
{
    return lzap_version();
}

JNIEXPORT jstring JNICALL Java_com_djpsoft_zap_plugin_zap_1jni_seed_1to_1address(
    JNIEnv* env, jobject thiz, jstring key, jchar network_byte)
{
    char* c_key = strdup((*env)->GetStringUTFChars(env, key, 0));
    char output[1024];
    lzap_seed_to_address(c_key, network_byte, output);
    return (*env)->NewStringUTF(env, output);
}
