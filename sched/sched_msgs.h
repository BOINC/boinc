#include "msg_log.h"

class SCHED_MSG_LOG : public MSG_LOG {
    int debug_level;
    const char* v_format_kind(int kind) const;
    bool v_message_wanted(int kind) const;
public:
    enum Kind {
        CRITICAL,
        NORMAL,
        DEBUG
    };
    SCHED_MSG_LOG(): MSG_LOG(stderr) {}
    void set_debug_level(int new_level) { debug_level = new_level; }
};

extern SCHED_MSG_LOG log_messages;
