#include <stdio.h>

#include "boinc_db.h"
#include "assimilate_handler.h"
#include "sched_util.h"

void assimilate_handler(
    WORKUNIT& wu, vector<RESULT>& results, RESULT& canonical_result
    )
{
    write_log(MSG_NORMAL, "assimilating WU %s\n", wu.name);
    if (wu.canonical_resultid) {
        write_log(MSG_NORMAL, "canonical result:\n%s", canonical_result.xml_doc_out);
    } else {
        write_log(MSG_NORMAL, "no canonical result\n");
    }
    if (wu.error_mask&WU_ERROR_COULDNT_SEND_RESULT) {
        write_log(MSG_NORMAL, "Error: couldn't send a result\n");
    }
    if (wu.error_mask&WU_ERROR_TOO_MANY_ERROR_RESULTS) {
        write_log(MSG_NORMAL, "Error: too many error results\n");
    }
    if (wu.error_mask&WU_ERROR_TOO_MANY_RESULTS) {
        write_log(MSG_NORMAL, "Error: too many total results\n");
    }
}
