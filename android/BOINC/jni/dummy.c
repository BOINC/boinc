#include <jni.h>
#include "include/dummy.h"

// this library's purpose is to trigger implicit architecture filters in GooglePlay.
// in order to build the library, get the Android NDK and run 
// ndk-build 
// in the root directory of this project.
 
JNIEXPORT jstring JNICALL Java_edu_berkeley_boinc_BOINCActivity_getDummyString
          (JNIEnv *env, jobject thisObj) {
   return (*env)->NewStringUTF(env, "Hello from native code!");
}
