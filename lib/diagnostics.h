// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2025 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

#ifndef BOINC_DIAGNOSTICS_H
#define BOINC_DIAGNOSTICS_H


#ifdef _WIN32
#include "boinc_win.h"
#else
#include <signal.h>
#ifdef __cplusplus
#include <cassert>
#else
#include <assert.h>
#endif
#endif

#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif

#ifdef __cplusplus
extern bool main_exited;
#endif

// some of the Android stuff below causes seg faults on some devices.
// Disable by default.
// Set this to enable it.
//
//#define ANDROID_VOODOO

// flags for boinc_init_diagnostics()
//
#define BOINC_DIAG_DUMPCALLSTACKENABLED     0x00000001L
#define BOINC_DIAG_HEAPCHECKENABLED         0x00000002L
#define BOINC_DIAG_MEMORYLEAKCHECKENABLED   0x00000004L
#define BOINC_DIAG_ARCHIVESTDERR            0x00000008L
#define BOINC_DIAG_ARCHIVESTDOUT            0x00000010L
#define BOINC_DIAG_REDIRECTSTDERR           0x00000020L
#define BOINC_DIAG_REDIRECTSTDOUT           0x00000040L
#define BOINC_DIAG_REDIRECTSTDERROVERWRITE  0x00000080L
#define BOINC_DIAG_REDIRECTSTDOUTOVERWRITE  0x00000100L
#define BOINC_DIAG_TRACETOSTDERR            0x00000200L
#define BOINC_DIAG_TRACETOSTDOUT            0x00000400L
#define BOINC_DIAG_HEAPCHECKEVERYALLOC      0x00000800L
#define BOINC_DIAG_BOINCAPPLICATION         0x00001000L
#define BOINC_DIAG_PERUSERLOGFILES          0x00002000L
#define BOINC_DIAG_DEFAULTS \
    BOINC_DIAG_DUMPCALLSTACKENABLED | \
    BOINC_DIAG_HEAPCHECKENABLED | \
    BOINC_DIAG_MEMORYLEAKCHECKENABLED | \
    BOINC_DIAG_REDIRECTSTDERR | \
    BOINC_DIAG_TRACETOSTDERR


// filenames
//
#define BOINC_DIAG_STDERR                  "stderr"
#define BOINC_DIAG_STDOUT                  "stdout"
#define BOINC_DIAG_GFX_STDERR              "stderrgfx"
#define BOINC_DIAG_GFX_STDOUT              "stdoutgfx"


#ifdef __cplusplus
extern "C" {
#endif


// These are functions common to all platforms
extern int boinc_init_diagnostics( int flags );
extern int boinc_init_graphics_diagnostics( int flags );
extern int boinc_install_signal_handlers(void);
extern int boinc_finish_diag(void);

extern int diagnostics_init(
    int flags, const char* stdout_prefix, const char* stderr_prefix
);
extern int diagnostics_thread_init(void);
extern int diagnostics_finish(void);
extern int diagnostics_is_initialized(void);
extern int diagnostics_is_flag_set(int flags);

// Properties
extern char* diagnostics_get_boinc_dir(void);
extern char* diagnostics_get_boinc_install_dir(void);
extern char* diagnostics_get_symstore(void);
extern int diagnostics_set_symstore(const char* symstore);
extern int diagnostics_is_proxy_enabled(void);
extern char* diagnostics_get_proxy(void);

extern int diagnostics_is_aborted_via_gui(void);
extern int diagnostics_set_aborted_via_gui(void);

// Log rotation
extern int diagnostics_cycle_logs(void);
extern void diagnostics_set_max_file_sizes(double stdout_size, double stderr_size);

// Thread Tracking
extern int diagnostics_init_thread_list(void);
extern int diagnostics_finish_thread_list(void);
extern int diagnostics_update_thread_list(void);
extern int diagnostics_set_thread_exempt_suspend(void);
extern int diagnostics_is_thread_exempt_suspend(long thread_id);

// Message Monitoring (debugger viewport)
extern int diagnostics_init_message_monitor(void);
extern int diagnostics_finish_message_monitor(void);
#ifdef _WIN32
extern UINT WINAPI diagnostics_message_monitor(LPVOID lpParameter);
#endif
extern int diagnostics_trace_to_debugger(const char* msg);

// Unhandled exception monitor
extern int diagnostics_init_unhandled_exception_monitor(void);
extern int diagnostics_finish_unhandled_exception_monitor(void);
#ifdef _WIN32
extern UINT WINAPI diagnostics_unhandled_exception_monitor(LPVOID lpParameter);
extern LONG CALLBACK boinc_catch_signal(EXCEPTION_POINTERS *ExceptionInfo);
#else
#ifdef HAVE_SIGACTION
typedef void (*handler_t)(int, siginfo_t*, void *);
extern void boinc_catch_signal(int signal, siginfo_t *siginfo, void *sigcontext);
#else
typedef void (*handler_t)(int);
extern void boinc_catch_signal(int signal);
#endif
extern void boinc_set_signal_handler(int sig, handler_t handler);
extern void boinc_set_signal_handler_force(int sig, handler_t handler);
#endif


// These functions are used to log the various messages that are
//   defined in the BOINC Diagnostics Library
extern void boinc_trace(const char *pszFormat, ...);
extern void boinc_info(const char *pszFormat, ...);

extern void set_signal_exit_code(int);

#ifdef __cplusplus
}
#endif

#ifdef ANDROID_VOODOO
// Yes, these are undocumented android functions located
// libcorkscrew.so .  They may not always be there, but it's better than
// nothing.  And we've got source so we could reimplement them if necessary.
extern const char *argv0;

typedef struct map_info_t map_info_t;

typedef struct {
    uintptr_t absolute_pc;
    uintptr_t stack_top;
    size_t stack_size;
} backtrace_frame_t;

typedef struct {
    uintptr_t relative_pc;
    uintptr_t relative_symbol_addr;
    char* map_name;
    char* symbol_name;
    char* demangled_name;
} backtrace_symbol_t;

typedef struct {
    uintptr_t start;
    uintptr_t end;
    char* name;
} symbol_t;

typedef struct {
    symbol_t* symbols;
    size_t num_symbols;
} symbol_table_t;


typedef ssize_t (*unwind_backtrace_signal_arch_t)(
        siginfo_t *, void *, const map_info_t *, backtrace_frame_t *,
        size_t , size_t
    );
extern unwind_backtrace_signal_arch_t unwind_backtrace_signal_arch;

typedef map_info_t *(*acquire_my_map_info_list_t)();
extern acquire_my_map_info_list_t acquire_my_map_info_list;

typedef void (*release_my_map_info_list_t)(map_info_t *);
extern release_my_map_info_list_t release_my_map_info_list;

typedef void (*get_backtrace_symbols_t)(
        const backtrace_frame_t *, size_t, backtrace_symbol_t *
    );
extern get_backtrace_symbols_t get_backtrace_symbols;

typedef void (*free_backtrace_symbols_t)(backtrace_symbol_t* symbols,
size_t frames);
extern free_backtrace_symbols_t free_backtrace_symbols;

typedef symbol_table_t *(*load_symbol_table_t)(const char *);
extern load_symbol_table_t load_symbol_table;

typedef void (*free_symbol_table_t)(symbol_table_t *);
extern free_symbol_table_t free_symbol_table;

typedef symbol_t *(*find_symbol_t)(const symbol_table_t *, uintptr_t );
extern find_symbol_t find_symbol;

typedef void (* format_backtrace_line_t)(unsigned, const backtrace_frame_t *, const backtrace_symbol_t *, char *, size_t);
extern format_backtrace_line_t format_backtrace_line;

#endif // ANDROID_VOODOO

#ifdef _WIN32

// Define macros for both debug and release builds.
//
// We are using the native debugging technology built into the Microsoft
//   C Runtime Libraries to trap and report the asserts and traces.
//

#ifdef _DEBUG

#if defined(WXDEBUG) || defined(WXNDEBUG)

// wxWidgets UI Framework
//

#define BOINCASSERT(expr)   wxASSERT(expr)
#ifdef _UNICODE
#define BOINCTRACE          __noop
#else
#define BOINCTRACE          wxLogDebug
#endif

#elif defined(_CONSOLE) && !(defined(__MINGW32__) || defined(__CYGWIN32__))

// Microsoft CRT
//

#define BOINCASSERT(expr)   _ASSERTE(expr)
#define BOINCTRACE          boinc_trace

#endif // _CONSOLE

#else  // _DEBUG

#if defined(__MINGW32__) || defined(__CYGWIN32__)
#define BOINCASSERT(expr)   assert(expr)
#define BOINCTRACE          boinc_trace
#else  // __MINGW32__
#define BOINCASSERT(expr)   __noop
#define BOINCTRACE          __noop
#endif // __MINGW32__

#endif // _DEBUG

#elif defined(__APPLE__)

#define BOINCTRACE(...)

#else

#ifdef _DEBUG

// Standard Frameworks
//

#define BOINCASSERT         assert
#define BOINCTRACE          boinc_trace

#else  // _DEBUG

#define BOINCASSERT(expr)
#ifndef IRIX
#if defined(__MINGW32__) || defined(__CYGWIN32__)
#define BOINCTRACE
#else
#define BOINCTRACE(...)
#endif
#endif

#endif // _DEBUG

#endif // ! _WIN32


// If none of the TRACE and INFO macros have been defined for
//   any existing framework to plug into, null them out.
// ASSERT is a special case and should point to the CRT version
//   if it hasn't already been redirected.

#ifndef BOINCASSERT
#define BOINCASSERT			assert
#endif

#ifndef BOINCTRACE
#define BOINCTRACE
#endif

#ifndef BOINCINFO
#define BOINCINFO			boinc_info
#endif

#endif
