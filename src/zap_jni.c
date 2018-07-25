#include <jni.h>

#include "zap.h"

int Java_com_djpsoft_zap_plugin_zap_1jni_version()
{
    return lzap_version();
}
