/** @file util.c
 *
 * Utilities common for the producer and consumer side JNI libraries
 *
 * Author: Gábor Gombás
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <dc_client.h>
#include "util.h"

#include <alloca.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**********************************************************************
 * Throwing exceptions
 */

static void throw_runtime(JNIEnv *env, jthrowable cause, const char *msg)
{
	jthrowable exc;
	jmethodID meth;
	jstring msgstr;
	jclass cls;
	int ret;

	if (jni_debug & DBG_EXCEPTION)
		DC_log(LOG_NOTICE, "JNI: Throwing RuntimeDCException: %s",
			msg);

	msgstr = (*env)->NewStringUTF(env, msg);
	if (!msgstr)
		return;
	ret = GET_CTOR(env, RuntimeDCException, &cls, &meth);
	if (ret)
		return;
	exc = (*env)->NewObject(env, cls, meth, msgstr, cause);
	if (!exc)
		return;
	(*env)->DeleteLocalRef(env, msgstr);
	(*env)->DeleteLocalRef(env, cls);

	(*env)->Throw(env, exc);
}

static void exception_runtime(JNIEnv *env, jobject cause, const char *fmt, ...)
{
	char msg[256];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(msg, sizeof(msg), fmt, ap);
	va_end(ap);

	throw_runtime(env, cause, msg);
}

void propagate_runtime(JNIEnv *env, const char *fmt, ...)
{
	jthrowable cause;
	char msg[256];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(msg, sizeof(msg), fmt, ap);
	va_end(ap);

	cause = (*env)->ExceptionOccurred(env);
	(*env)->ExceptionClear(env);

	throw_runtime(env, cause, msg);
}

void throw_exc(JNIEnv *env, const char *classname, const char *fmt, ...)
{
	char msg[256];
	va_list ap;
	jclass cls;

	va_start(ap, fmt);
	vsnprintf(msg, sizeof(msg), fmt, ap);
	va_end(ap);

	if (jni_debug & DBG_EXCEPTION)
	{
		const char *p;

		p = strrchr(classname, '/');
		if (!p)
			p = classname;
		DC_log(LOG_NOTICE, "JNI: Throwing %s: %s", p, msg);
	}

	cls = find_class(env, classname);
	if (!cls)
		return;
	(*env)->ThrowNew(env, cls, msg);
}

void throw_dcexc(JNIEnv *env, int code, const char *fmt, ...)
{
	jthrowable exc;
	jmethodID meth;
	jstring msgstr;
	char msg[256];
	va_list ap;
	jclass cls;
	int ret;

	va_start(ap, fmt);
	vsnprintf(msg, sizeof(msg), fmt, ap);
	va_end(ap);

	if (jni_debug & DBG_EXCEPTION)
		DC_log(LOG_NOTICE, "JNI: Throwing DCException: %s", msg);

	msgstr = (*env)->NewStringUTF(env, msg);
	if (!msgstr)
		return;
	ret = GET_CTOR(env, DCException, &cls, &meth);
	if (ret)
		return;
	exc = (*env)->NewObject(env, cls, meth, code, msgstr);
	if (!exc)
		return;
	(*env)->DeleteLocalRef(env, msgstr);
	(*env)->DeleteLocalRef(env, cls);

	(*env)->Throw(env, exc);
}

/**********************************************************************
 * Looking up classes and methods
 */

jclass find_class(JNIEnv *env, const char *classname)
{
	jclass cls;

	cls = (*env)->FindClass(env, classname);
	if (!cls)
	{
		unsigned i;
		char *p;

		p = alloca(strlen(classname) + 1);
		strcpy(p, classname);
		for (i = 0; p[i]; i++)
			if (p[i] == '/')
				p[i] = '.';
		DC_log(LOG_ERR, "JNI: Class %s was not found", p);

		/* If FindClass() did not throw an exception, do it now */
		if (!(*env)->ExceptionCheck(env))
		{
			jclass exc;
			exc = (*env)->FindClass(env,
				CLASS_ClassNotFoundException);
			if (exc)
				(*env)->ThrowNew(env, exc, p);
		}
	}

	return cls;
}

int get_constructor(JNIEnv *env, const char *classname, const char *signature,
	jclass *cls, jmethodID *meth)
{
	*cls = find_class(env, classname);
	if (!*cls)
		return -1;

	*meth = (*env)->GetMethodID(env, *cls, "<init>", signature);
	if (!*meth)
	{
		int i;
		char *buf;

		buf = malloc(strlen(classname) + 1);
		if (!buf)
			return -1;

		/* Convert the classname */
		strcpy(buf, classname);
		for (i = 0; buf[i]; i++)
			if (buf[i] == '/')
				buf[i] = '.';

		propagate_runtime(env, "Class %s has no constructor with "
			"signature %s", buf, signature);
		free(buf);

		return -1;
	}
	return 0;
}

jmethodID get_method_id(JNIEnv *env, jclass cls, const char *methodname,
	const char *signature)
{
	jthrowable excoccurred;
	const char *namestr;
	jmethodID meth;
	jclass clscls;
	jobject name;

	meth = (*env)->GetMethodID(env, cls, methodname, signature);
	if (meth)
		return meth;

	/* Now the failure path. First save & clear pending exceptions */
	excoccurred = (*env)->ExceptionOccurred(env);
	if (excoccurred)
		(*env)->ExceptionClear(env);

	/* Determine the class name */
	clscls = (*env)->GetObjectClass(env, cls);
	if (!clscls)
		return NULL;

	meth = (*env)->GetMethodID(env, clscls, "getName", SIG_Class_getName);
	if (!meth)
	{
		if ((*env)->ExceptionCheck(env))
		{
			(*env)->ExceptionDescribe(env);
			(*env)->ExceptionClear(env);
		}
		exception_runtime(env, excoccurred, "Failed to locate the "
			"getName() method of a java.lang.Class object");
		return NULL;
	}

	name = (*env)->CallObjectMethod(env, cls, meth);
	if (!name)
	{
		if ((*env)->ExceptionCheck(env))
		{
			(*env)->ExceptionDescribe(env);
			(*env)->ExceptionClear(env);
		}
		exception_runtime(env, excoccurred, "Failed to invoke the "
			"getName() method of a java.lang.Class object");
		return NULL;
	}

	namestr = (*env)->GetStringUTFChars(env, name, 0);
	if (!namestr)
	{
		propagate_runtime(env, "Failed to retrieve the name of a "
			"java.lang.Class object");
		return NULL;
	}

	DC_log(LOG_ERR, "JNI: Failed to locate method %s (signature: '%s') of "
		"class %s", methodname, signature, namestr);

	exception_runtime(env, excoccurred, "Method %s.%s() was not found",
		namestr, methodname);

	(*env)->ReleaseStringUTFChars(env, name, namestr);

	return NULL;
}

/**********************************************************************
 * Parse the value of dc.jni.debug
 */

int parse_debug_options(JNIEnv *env)
{
	jstring keystr, optstr;
	jmethodID meth;
	jclass cls;

	/* Check for properties */
	cls = find_class(env, CLASS_System);
	if (!cls)
	{
		fprintf(stderr, "Class java.lang.System was not found\n");
		return -1;
	}
	meth = (*env)->GetStaticMethodID(env, cls, "getProperty",
		SIG_System_getProperty);
	if (!meth)
	{
		fprintf(stderr, "Method java.lang.System.getProperty() "
			"was not found\n");
		return -1;
	}

	/* Look for & process dc.jni.debug */
	keystr = (*env)->NewStringUTF(env, "dc.jni.debug");
	if (!keystr)
	{
		fprintf(stderr, "Failed to instantiate String object\n");
		return -1;
	}
	optstr = (*env)->CallStaticObjectMethod(env, cls, meth, keystr);
	if ((*env)->ExceptionCheck(env))
	{
		fprintf(stderr, "Exception occurred while reading the "
			"properties\n");
		return -1;
	}
	if (optstr)
	{
		const char *options;
		char *opts, *p;

		options = (*env)->GetStringUTFChars(env, optstr, NULL);
		if (!options)
			return -1;

		opts = alloca(strlen(options + 1));
		strcpy(opts, options);

		while ((p = strsep(&opts, ",")))
		{
			if (!strcmp(p, "all"))
				jni_debug = (unsigned)-1;
			else if (!strcmp(p, "event"))
				jni_debug |= DBG_EVENT;
			else if (!strcmp(p, "noevent"))
				jni_debug &= ~DBG_EVENT;
			else if (!strcmp(p, "exception"))
				jni_debug |= DBG_EXCEPTION;
			else if (!strcmp(p, "noexception"))
				jni_debug &= ~DBG_EXCEPTION;
			else
				fprintf(stderr, "JNI: property "
					"dc.jni.debug contains an unknown "
					"token: %s\n", p);
		}
		(*env)->ReleaseStringUTFChars(env, optstr, options);
	}

	if (optstr)
		(*env)->DeleteLocalRef(env, optstr);
	(*env)->DeleteLocalRef(env, keystr);
	(*env)->DeleteLocalRef(env, cls);

	return 0;
}
