#include <stdarg.h>

#include "message.h"

// Takes a printf style formatted string, inserts the proper values,
// and passes it to show_message
// TODO: add translation functionality
//
void msg_printf(PROJECT *p, int priority, char *fmt, ...) {
    char        buf[512];
    va_list     ap;

    if (fmt == NULL) return;

    // Since Windows doesn't support vsnprintf, we have to do a
    // workaround to prevent buffer overruns
    //
    if (strlen(fmt) > 512) fmt[511] = '\0';
    va_start(ap, fmt); // Parses string for variables
    vsprintf(buf, fmt, ap); // And convert symbols To actual numbers
    va_end(ap); // Results are stored in text

    show_message(p, buf, priority);
}
