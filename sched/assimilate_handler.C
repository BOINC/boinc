#include <stdio.h>

#include "boinc_db.h"
#include "sched_msgs.h"
#include "sched_util.h"
#include "assimilate_handler.h"

using std::vector;

void assimilate_handler(
    WORKUNIT& wu, vector<RESULT>& results, RESULT& canonical_result
) {
    SCOPE_MSG_LOG scope_messages(log_messages, SCHED_MSG_LOG::NORMAL);
    scope_messages.printf("[%s] Assimilating\n", wu.name);
    if (wu.canonical_resultid) {
        scope_messages.printf("[%s] Found canonical result\n", wu.name);
        log_messages.printf_multiline(
            SCHED_MSG_LOG::DEBUG, canonical_result.xml_doc_out,
            "[%s] canonical result", wu.name
        );
    } else {
        scope_messages.printf("[%s] No canonical result\n", wu.name);
    }
    if (wu.error_mask&WU_ERROR_COULDNT_SEND_RESULT) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "[%s] Error: couldn't send a result\n", wu.name);
    }
    if (wu.error_mask&WU_ERROR_TOO_MANY_ERROR_RESULTS) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "[%s] Error: too many error results\n", wu.name);
    }
    if (wu.error_mask&WU_ERROR_TOO_MANY_TOTAL_RESULTS) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "[%s] Error: too many total results\n", wu.name);
    }
    if (wu.error_mask&WU_ERROR_TOO_MANY_SUCCESS_RESULTS) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "[%s] Error: too many success results\n", wu.name);
    }
}
