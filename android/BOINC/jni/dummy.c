#include <jni.h>
#include "include/dummy.h"
 
JNIEXPORT jstring JNICALL Java_edu_berkeley_boinc_BOINCActivity_getDummyString
          (JNIEnv *env, jobject thisObj) {
   return (*env)->NewStringUTF(env, "Hello from native code!");
}
