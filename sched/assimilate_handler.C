#include <stdio.h>

#include "boinc_db.h"
#include "assimilate_handler.h"
#include "sched_util.h"

void assimilate_handler(
    WORKUNIT& wu, vector<RESULT>& results, RESULT& canonical_result
    )
{
    log_messages.printf(SchedMessages::NORMAL, "[%s] Assimilating\n", wu.name);
    if (wu.canonical_resultid) {
        log_messages.printf(SchedMessages::NORMAL, "canonical result:\n");
        log_messages.printf_multiline(SchedMessages::NORMAL, canonical_result.xml_doc_out, "[%s] ", wu.name);
    } else {
        log_messages.printf(SchedMessages::NORMAL, "no canonical result\n");
    }
    if (wu.error_mask&WU_ERROR_COULDNT_SEND_RESULT) {
        log_messages.printf(SchedMessages::NORMAL, "Error: couldn't send a result\n");
    }
    if (wu.error_mask&WU_ERROR_TOO_MANY_ERROR_RESULTS) {
        log_messages.printf(SchedMessages::NORMAL, "Error: too many error results\n");
    }
    if (wu.error_mask&WU_ERROR_TOO_MANY_RESULTS) {
        log_messages.printf(SchedMessages::NORMAL, "Error: too many total results\n");
    }
}
