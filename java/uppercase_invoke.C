#include <jni.h>

 #define PATH_SEPARATOR ':'
 #define USER_CLASSPATH "."

/* invoker for uppercase java programm

!!! don't forget to include location of libjvm.so to LD_LIBRARY_PATH before start !!!

*/


 int main(int argc, char **argv) {
     JNIEnv *env;
     JavaVM *jvm;
     jint res;
     jclass cls;
     jmethodID mid;
     jstring jstr;
     jclass stringClass;
     jobjectArray args;

 #ifdef JNI_VERSION_1_2
     JavaVMInitArgs vm_args;
     JavaVMOption options[1];
     options[0].optionString = "-Djava.class.path=" USER_CLASSPATH;
     vm_args.version = 0x00010002;
     vm_args.options = options;
     vm_args.nOptions = 1;
     vm_args.ignoreUnrecognized = JNI_TRUE;
     /* Create the Java VM */
	printf("Invoke: creating JVM\n");
     res = JNI_CreateJavaVM(&jvm, (void**)&env, &vm_args);
 #else
     JDK1_1InitArgs vm_args;
     char classpath[1024];
     vm_args.version = 0x00010001;
     JNI_GetDefaultJavaVMInitArgs(&vm_args);
     /* Append USER_CLASSPATH to the default system class path */
     sprintf(classpath, "%s%c%s",
             vm_args.classpath, PATH_SEPARATOR, USER_CLASSPATH);
     vm_args.classpath = classpath;
     /* Create the Java VM */
	printf("Invoke: creating JVM\n");
     res = JNI_CreateJavaVM(&jvm, &env, &vm_args);
 #endif /* JNI_VERSION_1_2 */

     if (res < 0) {
         fprintf(stderr, "Invoke: Can't create Java Virtual Machine\n");
         return 1;
     }
     fprintf(stdout,"Invoke: searching java class UpperCase\n");
     cls = env->FindClass("UpperCase");
     if (cls == NULL) {
         fprintf(stderr,"Invoke: can't find java class UpperCase\n");
         goto destroy;
     }
     fprintf(stdout,"Invoke: searching method UpperCase.main() \n");
     mid = env->GetStaticMethodID(cls, "main", "([Ljava/lang/String;)V");
     if (mid == NULL) {
         fprintf(stderr,"Invoke: can't find method UpperCase.main() \n");
         goto destroy;
     }
     jstr = env->NewStringUTF( "-cpu_time10");
     if (jstr == NULL) {
         fprintf(stderr,"Invoke: can't create jstring\n");
         goto destroy;
     }
     stringClass = env->FindClass("java/lang/String");
     args = env->NewObjectArray( 1, stringClass, jstr);
     if (args == NULL) {
         fprintf(stderr,"Invoke: can't create args array\n");
         goto destroy;
     }
     fprintf(stdout,"Invoke: calling UpperCase.main() \n");
     env->CallStaticVoidMethod( cls, mid, args);

 destroy:
     if (env->ExceptionOccurred()) {
         env->ExceptionDescribe();
     }
     fprintf(stdout,"Invoke: calling DestroyJVM()\n");
     jvm->DestroyJavaVM();
     return 0;
 }
