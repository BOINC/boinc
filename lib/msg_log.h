#include <cstdio>
#include <cstdarg>

// the __attribute((format...)) tags are GCC extensions that let the compiler
// do like-checking on printf-like arguments
//
#if !defined(__GNUC__) && !defined(__attribute__)
#define __attribute__(x) /*nothing*/
#endif

#ifdef _USING_FCGI_
#define __attribute__(x) /*nothing*/
#endif

class MSG_LOG {
    int debug_level;
    int indent_level;
    char spaces[80];
    FILE* output;
public:

    MSG_LOG(FILE* output);
    void enter_level(int = 1);
    void leave_level() { enter_level(-1); }
    MSG_LOG& operator++() { enter_level(); return *this; }
    MSG_LOG& operator--() { leave_level(); return *this; }

    void printf(int kind, const char* format, ...) __attribute__ ((format (printf, 3, 4)));
    void printf_multiline(int kind, const char* str, const char* prefix_format, ...) __attribute__ ((format (printf, 4, 5)));
    void printf_file(int kind, const char* filename, const char* prefix_format, ...) __attribute__ ((format (printf, 4, 5)));
    void vprintf(int kind, const char* format, va_list va);
    void vprintf_multiline(int kind, const char* str, const char* prefix_format, va_list va);
    void vprintf_file(int kind, const char* filename, const char* prefix_format, va_list va);

protected:

    virtual const char* v_format_kind(int kind) const = 0;
    virtual bool v_message_wanted(int kind) const = 0;
};

// automatically ++/--MSG_LOG on scope entry / exit.
// See lib/msg_log.C for commentary
//
class SCOPE_MSG_LOG {
    MSG_LOG& messages;
    int kind;
public:
    SCOPE_MSG_LOG(MSG_LOG& messages_, int kind_) : messages(messages_), kind(kind_)
    { ++messages; }
    ~SCOPE_MSG_LOG() { --messages; }
    SCOPE_MSG_LOG& operator++() { ++messages; return *this; }
    SCOPE_MSG_LOG& operator--() { --messages; return *this; }

    void printf(const char* format, ...) __attribute__ ((format (printf, 2, 3)));
    void printf_multiline(const char* str, const char* prefix_format, ...) __attribute__ ((format (printf, 3, 4)));
    void printf_file(const char* filename, const char* prefix_format, ...) __attribute__ ((format (printf, 3, 4)));
};

