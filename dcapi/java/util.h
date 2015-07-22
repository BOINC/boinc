/** @file util.h
 *
 * Utilities common for the producer and consumer side JNI libraries
 *
 * Author: Gábor Gombás
 */

#ifndef UTIL_H
#define UTIL_H

#include "jni_proto.h"

#ifndef __GNUC__
#define __attribute__((x))
#endif

#define G_GNUC_UNUSED		__attribute__((__unused__))
#define G_GNUC_INTERNAL		__attribute__((__visibility__("internal")))
#define G_GNUC_PRINTF(x, y)	__attribute((__format__(__printf__, x, y)))

/**********************************************************************
 * Data type definitions
 */

/* This is used to obey strict aliasing rules and shut up gcc warnings */
union getenvarg
{
	JNIEnv			*env;
	void			*ptr;
};


/**********************************************************************
 * Constants
 */

/* Macros used for class definitions:
 *
 * CLASS_<cls>		The fully qualified name of the class (using '/',
 *			not '.')
 * CTOR_<cls>		The signature of the class' constructor
 * SIG_<cls>_<meth>	The signature of method <meth> of class <cls>
 */

/* Native Java classes */
#define CLASS_ArrayList \
	"java/util/ArrayList"
#define CTOR_ArrayList \
	"(I)V"
#define SIG_ArrayList_add \
	"(ILjava/lang/Object;)V"
#define CLASS_Boolean \
	"java/lang/Boolean"
#define CTOR_Boolean \
	"(Z)V"
#define SIG_Boolean_booleanValue \
	"()Z"
#define SIG_Class_getName \
	"()Ljava/lang/String;"
#define CLASS_ClassNotFoundException \
	"java/lang/ClassNotFoundException"
#define CLASS_Double \
	"java/lang/Double"
#define CTOR_Double \
	"(D)V"
#define SIG_Double_doubleValue \
	"()D"
#define CLASS_IllegalArgumentException \
	"java/lang/IllegalArgumentException"
#define CLASS_Integer \
	"java/lang/Integer"
#define CTOR_Integer \
	"(I)V"
#define SIG_Integer_intValue \
	"()I"
#define CLASS_Long \
	"java/lang/Long"
#define CTOR_Long \
	"(J)V"
#define SIG_Long_longValue \
	"()J"
#define CLASS_NullPointerException \
	"java/lang/NullPointerException"
#define CLASS_System \
	"java/lang/System"
#define SIG_System_getProperty \
	"(Ljava/lang/String;)Ljava/lang/String;"

/* Classes defined in package hu.sztaki.lpds.dc */
#define CLASS_ClientEvent \
	"hu/sztaki/lpds/dc/client/Event"
#define CTOR_ClientEvent \
	"(JI)V"
#define CLASS_ClientVersion \
	"hu/sztaki/lpds/dc/client/Version"
#define SIG_ClientVersion_getVersion \
	"()Ljava/lang/String;"
#define CLASS_DCException \
	"hu/sztaki/lpds/dc/DCException"
#define CTOR_DCException \
	"(ILjava/lang/String;)V"
#define CLASS_RuntimeDCException \
	"hu/sztaki/lpds/dc/RuntimeDCException"
#define CTOR_RuntimeDCException \
	"(Ljava/lang/String;Ljava/lang/Throwable;)V"


/* Debugging classes */
#define DBG_CONN		(1 << 0)
#define DBG_EVENT		(1 << 1)
#define DBG_EXCEPTION		(1 << 2)


/**********************************************************************
 * Macros
 */

#define RETURN_IF_EXCEPTION(env, x) do {\
	if ((*(env))->ExceptionCheck(env)) \
		return (x); \
	} while (0)

#define GET_CTOR(env, name, cls, meth) \
	get_constructor(env, CLASS_ ## name, CTOR_ ## name, cls, meth)
#define GET_METH(env, cls, clsname, methname) \
	get_method_id(env, cls, G_STRINGIFY(methname), \
		SIG_ ## clsname ## _ ## methname)


/**********************************************************************
 * Prototypes
 */

void propagate_runtime(JNIEnv *env, const char *fmt, ...)
	G_GNUC_PRINTF(2, 3) G_GNUC_INTERNAL;
void throw_exc(JNIEnv *env, const char *classname, const char *fmt, ...)
	G_GNUC_PRINTF(3, 4) G_GNUC_INTERNAL;
void throw_dcexc(JNIEnv *env, int code, const char *fmt, ...)
	G_GNUC_PRINTF(3, 4) G_GNUC_INTERNAL;

jclass find_class(JNIEnv *env, const char *classname) G_GNUC_INTERNAL;
int get_constructor(JNIEnv *env, const char *class, const char *args,
	jclass *cls, jmethodID *meth) G_GNUC_INTERNAL;
jmethodID get_method_id(JNIEnv *env, jclass cls, const char *methodname,
	const char *signature) G_GNUC_INTERNAL;
int parse_debug_options(JNIEnv *env) G_GNUC_INTERNAL;


/**********************************************************************
 * Global variables
 */

extern unsigned jni_debug G_GNUC_INTERNAL;

#endif /* UTIL_H */
