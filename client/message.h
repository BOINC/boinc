// Show a message, preceded by timestamp and project name
// priorities:

#define MSG_INFO    1
    // cmdline: write to stdout
    // GUI: write to msg window
#define MSG_ERROR   2
    // cmdline: write to stderr
    // GUI: write to msg window in bold or red

extern void show_message(PROJECT *p, char* message, int priority);
