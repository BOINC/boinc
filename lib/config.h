/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */


/* double-inclusion protection for config.h */
#ifndef BOINC_CONFIG_H
#define BOINC_CONFIG_H

/* Version defines are now in version.h */
#include "version.h"



/* Define to 1 if the xgetbv instruction can be used in inline assember */
#define ASM_SUPPORTS_XGETBV 1

/* Directory where BOINC executables will be installed */
#define BOINC_EXECPREFIX "/usr/local/bin"

/* Directory where BOINC libraries will be installed */
#define BOINC_LIBDIR "/usr/local/lib"

/* Directory tree where BOINC will be installed */
#define BOINC_PREFIX "/usr/local"

/* Define to 1 if CYGWIN should use the WIN32 GUI */
/* #undef CYGWIN_USE_WIN32 */

/* Define to 1 if compiling under OS X 10.3 or earlier */
/* #undef DARWIN_10_3 */

/* Define to the platform's shared library extension */
#define DLLEXT ".so"

/* Use the Apple OpenGL framework. */
/* #undef HAVE_APPLE_OPENGL_FRAMEWORK */

/* Define to 1 if you have the <arpa/inet.h> header file. */
#define HAVE_ARPA_INET_H 1

/* Define to 1 if you have the <avxintrin.h> header file. */
/* #undef HAVE_AVXINTRIN_H */

/* Define to 1 if the system has the type `CLIENT_ID'. */
/* #undef HAVE_CLIENT_ID */

/* Define to 1 if you have the <crtdbg.h> header file. */
/* #undef HAVE_CRTDBG_H */

/* Define to 1 if you have the `daemon' function. */
#define HAVE_DAEMON 1

/* Define to 1 if you have the <dbghelp.h> header file. */
/* #undef HAVE_DBGHELP_H */

/* Define to 1 if you have the <ddk/ntapi.h> header file. */
/* #undef HAVE_DDK_NTAPI_H */

/* Define to 1 if you have the declaration of `cpuid', and to 0 if you don't.
   */
#define HAVE_DECL_CPUID 0

/* Define to 1 if you have the declaration of `fpreset', and to 0 if you
   don't. */
#define HAVE_DECL_FPRESET 0

/* Define to 1 if you have the declaration of `tzname', and to 0 if you don't.
   */
/* #undef HAVE_DECL_TZNAME */

/* Define to 1 if you have the declaration of `xgetbv', and to 0 if you don't.
   */
#define HAVE_DECL_XGETBV 0

/* Define to 1 if you have the declaration of `_cpuid', and to 0 if you don't.
   */
#define HAVE_DECL__CPUID 0

/* Define to 1 if you have the declaration of `_fpreset', and to 0 if you
   don't. */
#define HAVE_DECL__FPRESET 0

/* Define to 1 if you have the declaration of `_xgetbv', and to 0 if you
   don't. */
#define HAVE_DECL__XGETBV 0

/* Define to 1 if you have the declaration of `__cpuid', and to 0 if you
   don't. */
#define HAVE_DECL___CPUID 0

/* Define to 1 if you have the declaration of `__xgetbv', and to 0 if you
   don't. */
#define HAVE_DECL___XGETBV 0

/* Define to 1 if you have the <delayimp.h> header file. */
/* #undef HAVE_DELAYIMP_H */

/* Define to 1 if you have the <dirent.h> header file. */
#define HAVE_DIRENT_H 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you don't have `vprintf' but do have `_doprnt.' */
/* #undef HAVE_DOPRNT */

/* Define to 1 if you have the <emmintrin.h> header file. */
#define HAVE_EMMINTRIN_H 1

/* Define to 1 if you have the <errno.h> header file. */
#define HAVE_ERRNO_H 1

/* Define to 1 if you have the `ether_ntoa' function. */
#define HAVE_ETHER_NTOA 1

/* Define to 1 if you have the <execinfo.h> header file. */
/* #undef HAVE_EXECINFO_H */

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define to 1 if you have the <float.h> header file. */
#define HAVE_FLOAT_H 1

/* Define to 1 if you have the `getisax' function. */
/* #undef HAVE_GETISAX */

/* Define to 1 if you have the `getutent' function. */
#define HAVE_GETUTENT 1

/* Define to 1 if you have the <glaux.h> header file. */
/* #undef HAVE_GLAUX_H */

/* Define to 1 if you have the <GLUT/glut.h> header file. */
/* #undef HAVE_GLUT_GLUT_H */

/* Define to 1 if you have the <glut.h> header file. */
/* #undef HAVE_GLUT_H */

/* Define to 1 if you have the <glu.h> header file. */
/* #undef HAVE_GLU_H */

/* Define to 1 if you have the <GL/glaux.h> header file. */
/* #undef HAVE_GL_GLAUX_H */

/* Define to 1 if you have the <GL/glut.h> header file. */
/* #undef HAVE_GL_GLUT_H */

/* Define to 1 if you have the <GL/glu.h> header file. */
/* #undef HAVE_GL_GLU_H */

/* Define to 1 if you have the <GL/gl.h> header file. */
/* #undef HAVE_GL_GL_H */

/* Define to 1 if you have the <gl.h> header file. */
/* #undef HAVE_GL_H */

/* Define to 1 if you have the `gmtime' function. */
#define HAVE_GMTIME 1

/* Define to 1 if you have the `gmtime_r' function. */
#define HAVE_GMTIME_R 1

/* Define to 1 if you have the <grp.h> header file. */
#define HAVE_GRP_H 1

/* Define to 1 if you have the <gtk/gtk.h> header file. */
/* #undef HAVE_GTK_GTK_H */

/* Define to 1 if you have the <ieeefp.h> header file. */
/* #undef HAVE_IEEEFP_H */

/* Define to 1 if you have the <immintrin.h> header file. */
#define HAVE_IMMINTRIN_H 1

/* Define to 1 if you have the <intrin.h> header file. */
/* #undef HAVE_INTRIN_H */

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the c library */
/* #undef HAVE_LIBC */

/* Define to 1 if you have a functional curl library. */
#define HAVE_LIBCURL 1

/* Define to 1 if you have the c_nonshared library */
/* #undef HAVE_LIBC_NONSHARED */

/* Define to 1 if you have the gcc library */
/* #undef HAVE_LIBGCC */

/* Define to 1 if you have the gcc_eh library */
/* #undef HAVE_LIBGCC_EH */

/* Define to 1 if you have the `gdi32' library (-lgdi32). */
/* #undef HAVE_LIBGDI32 */

/* Define to 1 if you have the iphlpapi library */
/* #undef HAVE_LIBIPHLPAPI */

/* Define to 1 if you have the math library */
#define HAVE_LIBM 1

/* Define to 1 if you have the msvcr100 library */
/* #undef HAVE_LIBMSVCR100 */

/* Define to 1 if you have the msvcr110 library */
/* #undef HAVE_LIBMSVCR110 */

/* Define to 1 if you have the msvcr90 library */
/* #undef HAVE_LIBMSVCR90 */

/* Define to 1 if you have the msvcr90d library */
/* #undef HAVE_LIBMSVCR90D */

/* Define to 1 if you have the <libnotify/notify.h> header file. */
/* #undef HAVE_LIBNOTIFY_NOTIFY_H */

/* Define to 1 if you have the NVIDIA API library */
/* #undef HAVE_LIBNVAPI */

/* Define to 1 if you have the pthread library */
/* #undef HAVE_LIBPTHREAD */

/* Define to 1 if you have the secur32 library */
/* #undef HAVE_LIBSECUR32 */

/* Define to 1 if you have the sensapi library */
/* #undef HAVE_LIBSENSAPI */

/* Define to 1 if you have the stdc++ library */
/* #undef HAVE_LIBSTDC__ */

/* Define to 1 if you have the userenv library */
/* #undef HAVE_LIBUSERENV */

/* Define to 1 if you have the gdi32 library */
/* #undef HAVE_LIBWGDI32 */

/* Define to 1 if you have the WinHttp library */
/* #undef HAVE_LIBWINHTTP */

/* Define to 1 if you have the wininet library */
/* #undef HAVE_LIBWININET */

/* Define to 1 if you have the `winmm' library (-lwinmm). */
/* #undef HAVE_LIBWINMM */

/* Define to 1 if you have the wsock32 library */
/* #undef HAVE_LIBWSOCK32 */

/* Define to 1 if you have the wtsapi32 library */
/* #undef HAVE_LIBWTSAPI32 */

/* Define to 1 if you have the `localtime' function. */
#define HAVE_LOCALTIME 1

/* Define to 1 if you have the `localtime_r' function. */
#define HAVE_LOCALTIME_R 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <MesaGL/glaux.h> header file. */
/* #undef HAVE_MESAGL_GLAUX_H */

/* Define to 1 if you have the <MesaGL/glut.h> header file. */
/* #undef HAVE_MESAGL_GLUT_H */

/* Define to 1 if you have the <MesaGL/glu.h> header file. */
/* #undef HAVE_MESAGL_GLU_H */

/* Define to 1 if you have the <MesaGL/gl.h> header file. */
/* #undef HAVE_MESAGL_GL_H */

/* Define to 1 if you have a visual c runtime library */
/* #undef HAVE_MSVCRT */

/* Define if your C++ compiler supports namespaces */
#define HAVE_NAMESPACES 1

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_NDIR_H */

/* Define to 1 if you have the <netdb.h> header file. */
#define HAVE_NETDB_H 1

/* Define to 1 if you have the <netinet/ether.h> header file. */
#define HAVE_NETINET_ETHER_H 1

/* Define to 1 if you have the <netinet/if_ether.h> header file. */
#define HAVE_NETINET_IF_ETHER_H 1

/* Define to 1 if you have the <netinet/in.h> header file. */
#define HAVE_NETINET_IN_H 1

/* Define to 1 if you have the <netinet/tcp.h> header file. */
#define HAVE_NETINET_TCP_H 1

/* Define to 1 if you have the <net/if_arp.h> header file. */
#define HAVE_NET_IF_ARP_H 1

/* Define to 1 if you have the <net/if.h> header file. */
#define HAVE_NET_IF_H 1

/* Define to 1 if you have the <ntapi.h> header file. */
/* #undef HAVE_NTAPI_H */

/* Define to 1 if your compiler has the nvapi.h header file */
/* #undef HAVE_NVAPI_H */

/* Define to 1 if you have the <OpenGL/glaux.h> header file. */
/* #undef HAVE_OPENGL_GLAUX_H */

/* Define to 1 if you have the <OpenGL/glut.h> header file. */
/* #undef HAVE_OPENGL_GLUT_H */

/* Define to 1 if you have the <OpenGL/glu.h> header file. */
/* #undef HAVE_OPENGL_GLU_H */

/* Define to 1 if you have the <OpenGL/gl.h> header file. */
/* #undef HAVE_OPENGL_GL_H */

/* Define to 1 if you have the <pmmintrin.h> header file. */
#define HAVE_PMMINTRIN_H 1

/* Define to 1 if you have the <procfs.h> header file. */
/* #undef HAVE_PROCFS_H */

/* Define if you have POSIX threads libraries and header files. */
#define HAVE_PTHREAD 1

/* Have PTHREAD_PRIO_INHERIT. */
#define HAVE_PTHREAD_PRIO_INHERIT 1

/* Define to 1 if you have the `putenv' function. */
#define HAVE_PUTENV 1

/* Define to 1 if you have the resolv library */
/* #undef HAVE_RESOLV */

/* Define to 1 if you have the `res_init' function. */
#define HAVE_RES_INIT 1

/* Define to 1 if you have the <sal.h> header file. */
/* #undef HAVE_SAL_H */

/* Define to 1 if you have the `sched_setscheduler' function. */
#define HAVE_SCHED_SETSCHEDULER 1

/* Define to 1 if your compiler has the security.h header file */
/* #undef HAVE_SECURITY_H */

/* Define to 1 if you have the `setenv' function. */
#define HAVE_SETENV 1

/* Define to 1 if you have the <setjmp.h> header file. */
#define HAVE_SETJMP_H 1

/* Define to 1 if you have the `setpriority' function. */
#define HAVE_SETPRIORITY 1

/* Define to 1 if you have the `setutent' function. */
#define HAVE_SETUTENT 1

/* Define to 1 if you have the `sigaction' function. */
#define HAVE_SIGACTION 1

/* Define to 1 if you have the <signal.h> header file. */
#define HAVE_SIGNAL_H 1

/* Define to 1 if the system has the type `socklen_t'. */
#define HAVE_SOCKLEN_T 1

/* Define to 1 if you have sqlite3. */
/* #undef HAVE_SQLITE3 */

/* Define to 1 if you have the `sqlite3_open' function. */
/* #undef HAVE_SQLITE3_OPEN */

/* Define to 1 if you have the `stat64' function. */
#define HAVE_STAT64 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if locale is in namespace std:: */
/* #undef HAVE_STD_LOCALE */

/* Define to 1 if max is in namespace std:: */
#define HAVE_STD_MAX 1

/* Define to 1 if min is in namespace std:: */
#define HAVE_STD_MIN 1

/* Define to 1 if transform is in namespace std:: */
#define HAVE_STD_TRANSFORM 1

/* Define to 1 if you have the `strcasecmp' function. */
#define HAVE_STRCASECMP 1

/* Define to 1 if you have the `strcasestr' function. */
#define HAVE_STRCASESTR 1

/* Define to 1 if you have the `strdup' function. */
#define HAVE_STRDUP 1

/* Define to 1 if you have the `strdupa' function. */
/* #undef HAVE_STRDUPA */

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strlcat' function. */
#define HAVE_STRLCAT 0

/* Define to 1 if you have the `strlcpy' function. */
#define HAVE_STRLCPY 0

/* Define to 1 if you have the `strtoull' function. */
#define HAVE_STRTOULL 1

/* Define to 1 if the system has the type `struct ether_addr'. */
/* #undef HAVE_STRUCT_ETHER_ADDR */

/* Define to 1 if the system has the type `struct ifconf'. */
#define HAVE_STRUCT_IFCONF 1

/* Define to 1 if the system has the type `struct ifreq'. */
#define HAVE_STRUCT_IFREQ 1

/* Define to 1 if the system has the type `struct lifconf'. */
/* #undef HAVE_STRUCT_LIFCONF */

/* Define to 1 if the system has the type `struct lifreq'. */
/* #undef HAVE_STRUCT_LIFREQ */

/* Define to 1 if `tm_zone' is a member of `struct tm'. */
#define HAVE_STRUCT_TM_TM_ZONE 1

/* Define to 1 if the system has the type `SYSTEM_PROCESSES'. */
/* #undef HAVE_SYSTEM_PROCESSES */

/* Define to 1 if the system has the type `SYSTEM_THREADS'. */
/* #undef HAVE_SYSTEM_THREADS */

/* Define to 1 if you have the <sys/auxv.h> header file. */
#define HAVE_SYS_AUXV_H 1

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_DIR_H */

/* Define to 1 if you have the <sys/fcntl.h> header file. */
#define HAVE_SYS_FCNTL_H 1

/* Define to 1 if you have the <sys/file.h> header file. */
#define HAVE_SYS_FILE_H 1

/* Define to 1 if you have the <sys/ioctl.h> header file. */
#define HAVE_SYS_IOCTL_H 1

/* Define to 1 if you have the <sys/ipc.h> header file. */
#define HAVE_SYS_IPC_H 1

/* Define to 1 if you have the <sys/mount.h> header file. */
#define HAVE_SYS_MOUNT_H 1

/* Define to 1 if you have the <sys/msg.h> header file. */
#define HAVE_SYS_MSG_H 1

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_NDIR_H */

/* Define to 1 if you have the <sys/param.h> header file. */
#define HAVE_SYS_PARAM_H 1

/* Define to 1 if you have the <sys/resource.h> header file. */
#define HAVE_SYS_RESOURCE_H 1

/* Define to 1 if you have the <sys/select.h> header file. */
#define HAVE_SYS_SELECT_H 1

/* Define to 1 if you have the <sys/sem.h> header file. */
#define HAVE_SYS_SEM_H 1

/* Define to 1 if you have the <sys/sensors.h> header file. */
/* #undef HAVE_SYS_SENSORS_H */

/* Define to 1 if you have the <sys/shm.h> header file. */
#define HAVE_SYS_SHM_H 1

/* Define to 1 if you have the <sys/socket.h> header file. */
#define HAVE_SYS_SOCKET_H 1

/* Define to 1 if you have the <sys/sockio.h> header file. */
/* #undef HAVE_SYS_SOCKIO_H */

/* Define to 1 if you have the <sys/statfs.h> header file. */
#define HAVE_SYS_STATFS_H 1

/* Define to 1 if you have the <sys/statvfs.h> header file. */
#define HAVE_SYS_STATVFS_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/swap.h> header file. */
#define HAVE_SYS_SWAP_H 1

/* Define to 1 if you have the <sys/sysctl.h> header file. */
/* #undef HAVE_SYS_SYSCTL_H */

/* Define to 1 if you have the <sys/systeminfo.h> header file. */
/* #undef HAVE_SYS_SYSTEMINFO_H */

/* Define to 1 if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <sys/un.h> header file. */
#define HAVE_SYS_UN_H 1

/* Define to 1 if you have the <sys/utsname.h> header file. */
#define HAVE_SYS_UTSNAME_H 1

/* Define to 1 if you have the <sys/vmmeter.h> header file. */
/* #undef HAVE_SYS_VMMETER_H */

/* Define to 1 if you have the <sys/wait.h> header file. */
#define HAVE_SYS_WAIT_H 1

/* Define to 1 if the system has the type `THREAD_STATE'. */
/* #undef HAVE_THREAD_STATE */

/* Define to 1 if the system has the type `THREAD_WAIT_REASON'. */
/* #undef HAVE_THREAD_WAIT_REASON */

/* Define to 1 if your `struct tm' has `tm_zone'. Deprecated, use
   `HAVE_STRUCT_TM_TM_ZONE' instead. */
#define HAVE_TM_ZONE 1

/* Define to 1 if you don't have `tm_zone' but do have the external array
   `tzname'. */
/* #undef HAVE_TZNAME */

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the `unsetenv' function. */
#define HAVE_UNSETENV 1

/* Define to 1 if you have the `uselocale' function. */
#define HAVE_USELOCALE 1

/* Define to 1 if you have the <utmp.h> header file. */
#define HAVE_UTMP_H 1

/* Define to 1 if the system has the type `VM_COUNTERS'. */
/* #undef HAVE_VM_COUNTERS */

/* Define to 1 if you have the `vprintf' function. */
#define HAVE_VPRINTF 1

/* Define to 1 if you have the <windows.h> header file. */
/* #undef HAVE_WINDOWS_H */

/* Define to 1 if you have the <winhttp.h> header file. */
/* #undef HAVE_WINHTTP_H */

/* Define to 1 if you have the <winsock2.h> header file. */
/* #undef HAVE_WINSOCK2_H */

/* Define to 1 if you have the <winsock.h> header file. */
/* #undef HAVE_WINSOCK_H */

/* Define to 1 if you have the <winternl.h> header file. */
/* #undef HAVE_WINTERNL_H */

/* Define to 1 if you have the <ws2tcpip.h> header file. */
/* #undef HAVE_WS2TCPIP_H */

/* Define to 1 if you have X11 library */
/* #undef HAVE_X11 */

/* Define to 1 if you have the <x86intrin.h> header file. */
#define HAVE_X86INTRIN_H 1

/* Define to 1 if you have the <xlocale.h> header file. */
#define HAVE_XLOCALE_H 1

/* Define to 1 if you have the <xmmintrin.h> header file. */
#define HAVE_XMMINTRIN_H 1

/* Define to 1 if you have xss library */
/* #undef HAVE_XSS */

/* Define to 1 if you have the `_configthreadlocale' function. */
/* #undef HAVE__CONFIGTHREADLOCALE */

/* Define to 1 if /proc/meminfo exists */
#define HAVE__PROC_MEMINFO 1

/* Define to 1 if /proc/self/psinfo exists */
/* #undef HAVE__PROC_SELF_PSINFO */

/* Define to 1 if /proc/self/stat exists */
#define HAVE__PROC_SELF_STAT 1

/* Define to 1 if you have the `_strdup' function. */
/* #undef HAVE__STRDUP */

/* Define to 1 if you have the `_strdupa' function. */
/* #undef HAVE__STRDUPA */

/* Platform identification used to identify applications for this BOINC core
   client */
#define HOSTTYPE "x86_64-android-linux-gnu"

/* Alternate identification used to identify applications for this BOINC core
   client */
#define HOSTTYPEALT "x86-android-linux-gnu"

/* "Define to 1 if largefile support causes missing symbols in C++" */
#define LARGEFILE_BREAKS_CXX 1

/* Define to the name of libcurl's certification file */
#define LIBCURL_CABUNDLE ""

/* Defined if libcurl supports AsynchDNS */
#define LIBCURL_FEATURE_ASYNCHDNS 1

/* Defined if libcurl supports IPv6 */
#define LIBCURL_FEATURE_IPV6 1

/* Defined if libcurl supports KRB4 */
/* #undef LIBCURL_FEATURE_KRB4 */

/* Defined if libcurl supports libz */
#define LIBCURL_FEATURE_LIBZ 1

/* Defined if libcurl supports SSL */
#define LIBCURL_FEATURE_SSL 1

/* Defined if libcurl supports DICT */
#define LIBCURL_PROTOCOL_DICT 1

/* Defined if libcurl supports FILE */
#define LIBCURL_PROTOCOL_FILE 1

/* Defined if libcurl supports FTP */
#define LIBCURL_PROTOCOL_FTP 1

/* Defined if libcurl supports FTPS */
#define LIBCURL_PROTOCOL_FTPS 1

/* Defined if libcurl supports GOPHER */
#define LIBCURL_PROTOCOL_GOPHER 1

/* Defined if libcurl supports HTTP */
#define LIBCURL_PROTOCOL_HTTP 1

/* Defined if libcurl supports HTTPS */
#define LIBCURL_PROTOCOL_HTTPS 1

/* Defined if libcurl supports LDAP */
/* #undef LIBCURL_PROTOCOL_LDAP */

/* Defined if libcurl supports TELNET */
#define LIBCURL_PROTOCOL_TELNET 1

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#define LT_OBJDIR ".libs/"

/* Name of package */
#define PACKAGE "boinc"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME "BOINC"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "BOINC 7.17.0"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "boinc"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "7.17.0"

/* Define to necessary symbol if this constant uses a non-standard name on
   your system. */
/* #undef PTHREAD_CREATE_JOINABLE */

/* Define as the return type of signal handlers (`int' or `void'). */
#define RETSIGTYPE void

/* Define to 1 if your compiler has the security.h header file */
/* #undef SECURITY_WIN32 */

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#define TIME_WITH_SYS_TIME 1

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
/* #undef TM_IN_SYS_TIME */

/* "Define to 1 if you want to use the openssl crypto library" */
#define USE_OPENSSL 1

/* Define to 1 to use windows sockets */
/* #undef USE_WINSOCK */

/* utmp file location */
/* #undef UTMP_LOCATION */

/* Version number of package */
#define VERSION "7.17.0"

/* Define to prevent redefinition of INT32 in jconfig.h */
/* #undef XMD_H */

/* Define to 1 if the X Window System is missing or not being used. */
#define X_DISPLAY_MISSING 1

/* Enable large inode numbers on Mac OS X 10.5.  */
#ifndef _DARWIN_USE_64_BIT_INODE
# define _DARWIN_USE_64_BIT_INODE 1
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
/* #undef _FILE_OFFSET_BITS */

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define curl_free() as free() if our version of curl lacks curl_free. */
/* #undef curl_free */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */



#ifndef HAVE_RES_INIT
#define res_init() (0)
#endif

#include "project_specific_defines.h"

/* end double-inclusion protection for config.h */
#endif /* #ifndef BOINC_CONFIG_H */

