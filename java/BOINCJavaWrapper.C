
#include "jni.h"
#include "boinc_api.h"
#include "diagnostics.h"

extern "C" {
JNIEXPORT jstring JNICALL Java_BOINCJavaWrapper_resolve_1filename(JNIEnv * env, jobject obj, jstring symName) {
	env->MonitorEnter(obj);
	jint ret=0;
	jboolean iscopy;
	jstring physName=0;
	char c_physName[512];
	const char *c_symName = env->GetStringUTFChars(symName, &iscopy);
	printf("JNI: calling boinc_resolve_filename()\n");
	ret = boinc_resolve_filename(c_symName,c_physName,sizeof(c_physName));
	printf("JNI: returned from boinc_resolve_filename with %d\n",ret);
	printf("JNI: allocating memory for physical file name %s\n",c_physName);
	physName = env->NewStringUTF(c_physName);
	env->ReleaseStringUTFChars(symName,c_symName);
	env->MonitorExit(obj);
	return physName;
}

JNIEXPORT jint JNICALL Java_BOINCJavaWrapper_init(JNIEnv * env, jobject obj) {
	env->MonitorEnter(obj);
	printf("JNI: calling boinc_init()\n");
	jint res = boinc_init();
	printf("JNI: returned from boinc_init() with %d\n",res);
	env->MonitorExit(obj);
	return res;
}

JNIEXPORT jint JNICALL Java_BOINCJavaWrapper_finish(JNIEnv * env, jobject obj, jint status) {
	printf("JNI: calling boinc_finish(%d)\n",status);
	jint res = boinc_finish(status);
	//should never happen ?
	printf("JNI: returned from boinc_finish() with %d\n",res);
	return res;
}

JNIEXPORT jint JNICALL Java_BOINCJavaWrapper_fraction_1done(JNIEnv *env, jobject obj, jdouble fraction) {
	env->MonitorEnter(obj);
	printf("JNI: calling boinc_fraction_done()\n");
	jint res = boinc_fraction_done(fraction);
	printf("JNI: returned from boinc_fraction_done() with %d\n",res);
	env->MonitorExit(obj);
	return res;
}

JNIEXPORT jint JNICALL Java_BOINCJavaWrapper_init_1diagnostics(JNIEnv *env, jobject obj, jint flags) {
	env->MonitorEnter(obj);
	printf("JNI: calling boinc_init_diagnostics()\n");
	jint res = boinc_init_diagnostics(flags);
	printf("JNI: returned from boinc_init_diagnostics with %d\n",res);
	env->MonitorExit(obj);
	return res;
}

JNIEXPORT jboolean JNICALL Java_BOINCJavaWrapper_time_1to_1checkpoint (JNIEnv *env, jobject obj) {
	env->MonitorEnter(obj);
	printf("JNI: calling boinc_time_to_checkpoint()\n");
	jint res = boinc_time_to_checkpoint();
	printf("JNI: returned from boinc_time_to_checkpoint with %d\n",res);
	env->MonitorExit(obj);
	return res;
}

JNIEXPORT jint JNICALL Java_BOINCJavaWrapper_checkpoint_1completed(JNIEnv *env, jobject obj) {
	env->MonitorEnter(obj);
	printf("JNI: calling boinc_checkpoint_completed()\n");
	jint res = boinc_checkpoint_completed();
	printf("JNI: returned from boinc_checkpoint_cpmleted() with %d\n",res);
	env->MonitorExit(obj);
	return res;
}



}
