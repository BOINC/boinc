/** @file client.c
 *
 * Client-side DC-API JNI code
 *
 * Author: Gábor Gombás
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>

#include <dc_client.h>

#include "util.h"

/**********************************************************************
 * Global variables
 */

/* Java VM handle for the callbacks */
static JavaVM *myvm;

/* Bitmask for debugging classes */
unsigned jni_debug = 0;


/**********************************************************************
 * Class: hu.sztaki.lpds.dc.client.Event
 */

/* Throws: RuntimeDCException, DCException */
JNIEXPORT jstring JNICALL Java_hu_sztaki_lpds_dc_client_Event_getMessage
  (JNIEnv *env, jobject this)
{
	DC_ClientEvent *event;
	jfieldID field;
	jclass cls;
	int type;

	cls = (*env)->GetObjectClass(env, this);
	if (!cls)
		return NULL;

	field = (*env)->GetFieldID(env, cls, "type", "I");
	if (!field)
	{
		propagate_runtime(env, "Failed to look up the type field");
		return NULL;
	}

	type = (*env)->GetIntField(env, this, field);
	if (type != DC_CLIENT_MESSAGE)
	{
		throw_dcexc(env, DC_ERR_BADPARAM, "This is not a message event");
		return NULL;
	}

	field = (*env)->GetFieldID(env, cls, "ptr", "J");
	if (!field)
	{
		propagate_runtime(env, "Failed to look up the type field");
		return NULL;
	}

	event = (DC_ClientEvent *)(*env)->GetLongField(env, this, field);
	if (!event || !event->message)
	{
		throw_exc(env, CLASS_RuntimeDCException, "Empty message event?!?");
		return NULL;
	}

	return (*env)->NewStringUTF(env, event->message);
}

/* Throws: RuntimeDCException */
JNIEXPORT void JNICALL Java_hu_sztaki_lpds_dc_client_Event_finalize
  (JNIEnv *env, jobject this)
{
	DC_ClientEvent *event;
	jfieldID field;
	jclass cls;

	cls = (*env)->GetObjectClass(env, this);
	if (!cls)
		return;

	field = (*env)->GetFieldID(env, cls, "ptr", "J");
	if (!field)
	{
		propagate_runtime(env, "Failed to look up the type field");
		return;
	}

	event = (DC_ClientEvent *)(*env)->GetLongField(env, this, field);
	DC_destroyClientEvent(event);
}

/**********************************************************************
 * Class: hu.sztaki.lpds.dc.DCClient
 */

/* Throws: none */
JNIEXPORT jint JNICALL Java_hu_sztaki_lpds_dc_client_DCClient_getMaxMessageSize
  (JNIEnv *env G_GNUC_UNUSED, jclass cls G_GNUC_UNUSED)
{
	return DC_getMaxMessageSize();
}

/* Throws: none */
JNIEXPORT jint JNICALL Java_hu_sztaki_lpds_dc_client_DCClient_getMaxSubresults
  (JNIEnv *env G_GNUC_UNUSED, jclass cls G_GNUC_UNUSED)
{
	return DC_getMaxSubresults();
}

/* Throws: NullPointerException */
JNIEXPORT void JNICALL Java_hu_sztaki_lpds_dc_client_DCClient_log
  (JNIEnv *env, jclass cls G_GNUC_UNUSED, jint level, jstring msgstr)
{
	const char *msg;

	if (!msgstr)
	{
		throw_exc(env, CLASS_NullPointerException,
			"Message cannot be null");
		return;
	}

	msg = (*env)->GetStringUTFChars(env, msgstr, NULL);
	if (!msg)
		return;

	DC_log(level, "%s", msg);
	(*env)->ReleaseStringUTFChars(env, msgstr, msg);
}

/* Throws: NullPointerException, IllegalArgumentException */
JNIEXPORT jstring JNICALL Java_hu_sztaki_lpds_dc_client_DCClient_getCfgStr
  (JNIEnv *env, jclass cls G_GNUC_UNUSED, jstring namestr)
{
	const char *name;
	jstring res;
	char *value;

	if (!namestr)
	{
		throw_exc(env, CLASS_NullPointerException,
			"Configuration key name cannot be null");
		return NULL;
	}

	name = (*env)->GetStringUTFChars(env, namestr, NULL);
	if (!name)
		return NULL;

	value = DC_getCfgStr(name);
	if (!value)
	{
		throw_exc(env, CLASS_IllegalArgumentException,
			"Configuration key %s does not exist", name);
		(*env)->ReleaseStringUTFChars(env, namestr, name);
		return NULL;
	}

	res = (*env)->NewStringUTF(env, value);
	free(value);
	(*env)->ReleaseStringUTFChars(env, namestr, name);

	return res;
}

/* Throws: NullPointerException */
JNIEXPORT jint JNICALL Java_hu_sztaki_lpds_dc_client_DCClient_getCfgInt
  (JNIEnv *env, jclass cls G_GNUC_UNUSED, jstring namestr, jint defaultValue)
{
	const char *name;
	jint res;

	if (!namestr)
	{
		throw_exc(env, CLASS_NullPointerException,
			"Configuration key name cannot be null");
		return defaultValue;
	}

	name = (*env)->GetStringUTFChars(env, namestr, NULL);
	if (!name)
		return defaultValue;

	res = DC_getCfgInt(name, defaultValue);
	(*env)->ReleaseStringUTFChars(env, namestr, name);

	return res;
}

/* Throws: NullPointerException */
JNIEXPORT jboolean JNICALL Java_hu_sztaki_lpds_dc_client_DCClient_getCfgBool
  (JNIEnv *env, jclass cls G_GNUC_UNUSED, jstring namestr, jboolean defaultValue)
{
	const char *name;
	jboolean res;

	if (!namestr)
	{
		throw_exc(env, CLASS_NullPointerException,
			"Configuration key name cannot be null");
		return defaultValue;
	}

	name = (*env)->GetStringUTFChars(env, namestr, NULL);
	if (!name)
		return defaultValue;

	res = DC_getCfgBool(name, defaultValue);
	(*env)->ReleaseStringUTFChars(env, namestr, name);

	return res;
}

/* Throws: DCException */
JNIEXPORT void JNICALL Java_hu_sztaki_lpds_dc_client_DCClient_init
  (JNIEnv *env, jclass cls G_GNUC_UNUSED)
{
	int ret;

	ret = DC_initClient();
	if (ret)
		throw_dcexc(env, ret, "Failed to initialize the DC-API library");
}

/* Throws: DCException, NullPointerException */
JNIEXPORT jstring JNICALL Java_hu_sztaki_lpds_dc_client_DCClient_resolveFileName
  (JNIEnv *env, jclass cls G_GNUC_UNUSED, jobject typeobj, jstring namestr)
{
	DC_FileType type;
	const char *name;
	jfieldID field;
	jstring res;
	char *pname;

	if (!typeobj)
	{
		throw_exc(env, CLASS_NullPointerException,
			"The file type can not be null");
		return NULL;
	}
	if (!namestr)
	{
		throw_exc(env, CLASS_NullPointerException,
			"The file name can not be null");
		return NULL;
	}

	cls = (*env)->GetObjectClass(env, typeobj);
	if (!cls)
		return NULL;

	field = (*env)->GetFieldID(env, cls, "code", "I");
	if (!field)
	{
		propagate_runtime(env, "Failed to look up the code field");
		return NULL;
	}

	type = (*env)->GetIntField(env, typeobj, field);
	name = (*env)->GetStringUTFChars(env, namestr, NULL);
	pname = DC_resolveFileName(type, name);
	(*env)->ReleaseStringUTFChars(env, namestr, name);
	if (!pname)
	{
		throw_dcexc(env, DC_ERR_BADPARAM, "File name lookup failed");
		return NULL;
	}

	res = (*env)->NewStringUTF(env, pname);
	free(pname);
	return res;
}

JNIEXPORT void JNICALL Java_hu_sztaki_lpds_dc_client_DCClient_sendResult
  (JNIEnv *env, jclass cls G_GNUC_UNUSED, jstring namestr, jstring pathstr, jobject modeobj)
{
	const char *name, *path;
	DC_FileMode mode;
	jfieldID field;
	int ret;

	if (!modeobj)
	{
		throw_exc(env, CLASS_NullPointerException,
			"The file mode can not be null");
		return;
	}
	if (!namestr)
	{
		throw_exc(env, CLASS_NullPointerException,
			"The logical file name can not be null");
		return;
	}
	if (!pathstr)
	{
		throw_exc(env, CLASS_NullPointerException,
			"The physical file name can not be null");
		return;
	}

	cls = (*env)->GetObjectClass(env, modeobj);
	if (!cls)
		return;

	field = (*env)->GetFieldID(env, cls, "code", "I");
	if (!field)
	{
		propagate_runtime(env, "Failed to look up the code field");
		return;
	}

	mode = (*env)->GetIntField(env, modeobj, field);
	name = (*env)->GetStringUTFChars(env, namestr, NULL);
	path = (*env)->GetStringUTFChars(env, pathstr, NULL);

	ret = DC_sendResult(name, path, mode);
	(*env)->ReleaseStringUTFChars(env, namestr, name);
	(*env)->ReleaseStringUTFChars(env, pathstr, path);
	if (ret)
		throw_dcexc(env, ret, "Sub-result sending failed");
}

/* Throws: NullPointerException */
JNIEXPORT void JNICALL Java_hu_sztaki_lpds_dc_client_DCClient_sendMessage
  (JNIEnv *env, jclass cls G_GNUC_UNUSED, jstring msgstr)
{
	const char *msg;

	if (!msgstr)
	{
		throw_exc(env, CLASS_NullPointerException,
			"The message can not be null");
		return;
	}

	msg = (*env)->GetStringUTFChars(env, msgstr, NULL);
	DC_sendMessage(msg);
	(*env)->ReleaseStringUTFChars(env, msgstr, msg);
}

/* Throws: none */
JNIEXPORT void JNICALL Java_hu_sztaki_lpds_dc_client_DCClient_finish
  (JNIEnv *env G_GNUC_UNUSED, jclass cls G_GNUC_UNUSED, jint exitCode)
{
	DC_finishClient(exitCode);
}

/* Throws: NullPointerException */
JNIEXPORT void JNICALL Java_hu_sztaki_lpds_dc_client_DCClient_checkpointMade
  (JNIEnv *env, jclass cls G_GNUC_UNUSED, jstring pathstr)
{
	const char *path;

	if (!pathstr)
	{
		throw_exc(env, CLASS_NullPointerException,
			"The file name can not be null");
		return;
	}

	path = (*env)->GetStringUTFChars(env, pathstr, NULL);
	DC_checkpointMade(path);
	(*env)->ReleaseStringUTFChars(env, pathstr, path);
}

/* Throws: none */
JNIEXPORT void JNICALL Java_hu_sztaki_lpds_dc_client_DCClient_fractionDone
  (JNIEnv *env G_GNUC_UNUSED, jclass cls G_GNUC_UNUSED, jdouble fraction G_GNUC_UNUSED)
{
	DC_fractionDone(fraction);
}

/* Throws: none */
JNIEXPORT jobject JNICALL Java_hu_sztaki_lpds_dc_client_DCClient_checkEvent
  (JNIEnv *env, jclass cls G_GNUC_UNUSED)
{
	DC_ClientEvent *event;
	jobject eventobj;
	jmethodID meth;
	int ret;

	event = DC_checkClientEvent();
	if (!event)
		return NULL;

	ret = GET_CTOR(env, ClientEvent, &cls, &meth);
	if (ret)
		return NULL;

	eventobj = (*env)->NewObject(env, cls, meth, event->type, (jlong)event);
	return eventobj;
}

/**********************************************************************
 * Class: hu.sztaki.lpds.dc.client.Version
 */

JNIEXPORT jstring JNICALL Java_hu_sztaki_lpds_dc_client_Version_getNativeVersion
  (JNIEnv *env, jclass cls G_GNUC_UNUSED)
{
	return (*env)->NewStringUTF(env, PACKAGE_VERSION);
}

/**********************************************************************
 * JNI module initialization and finalization
 */

JNIEXPORT jint JNI_OnLoad(JavaVM *jvm, void *res G_GNUC_UNUSED)
{
	union getenvarg jnienv;
	const char *ver;
	jmethodID meth;
	jstring verstr;
	jclass cls;
	int ret;

	myvm = jvm;

	if ((*myvm)->GetEnv(myvm, &jnienv.ptr, JNI_VERSION_1_2))
		return 0;

	/* Check that the JNI and Java versions match */
	cls = find_class(jnienv.env, CLASS_ClientVersion);
	if (!cls)
	{
		fprintf(stderr, "Class hu.sztaki.lpds.dc.Version was "
			"not found\n");
		return 0;
	}
	meth = (*jnienv.env)->GetStaticMethodID(jnienv.env, cls, "getVersion",
		SIG_ClientVersion_getVersion);
	if (!meth)
	{
		fprintf(stderr, "Method hu.sztaki.lpds.dc.Version."
			"getVersion() was not found\n");
		return 0;
	}
	verstr = (*jnienv.env)->CallStaticObjectMethod(jnienv.env, cls, meth);
	if ((*jnienv.env)->ExceptionCheck(jnienv.env))
	{
		fprintf(stderr, "Exception occurred while checking the "
			"version\n");
		return 0;
	}
	ver = (*jnienv.env)->GetStringUTFChars(jnienv.env, verstr, NULL);
	if (!ver)
	{
		fprintf(stderr, "Failed to retrieve the version string\n");
		return 0;
	}
	if (strcmp(ver, PACKAGE_VERSION))
	{
		fprintf(stderr, "DC-API version mismatch: Java side is %s, "
			"JNI side is %s\n", ver, PACKAGE_VERSION);
		(*jnienv.env)->ReleaseStringUTFChars(jnienv.env, verstr, ver);
		return 0;
	}
	(*jnienv.env)->ReleaseStringUTFChars(jnienv.env, verstr, ver);

	(*jnienv.env)->DeleteLocalRef(jnienv.env, verstr);
	(*jnienv.env)->DeleteLocalRef(jnienv.env, cls);

	/* Look for & process dc.jni.debug */
	ret = parse_debug_options(jnienv.env);
	if (ret)
		return 0;

	return JNI_VERSION_1_2;
}

JNIEXPORT void JNI_OnUnload(JavaVM *jvm G_GNUC_UNUSED,
	void *res G_GNUC_UNUSED)
{
}
