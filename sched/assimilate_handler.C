#include <stdio.h>

#include "boinc_db.h"
#include "assimilate_handler.h"

void assimilate_handler(
    WORKUNIT& wu, vector<RESULT>& results, RESULT& canonical_result
) {
    printf("assimilating WU %s\n", wu.name);
    if (wu.canonical_resultid) {
        printf("canonical result:\n%s", canonical_result.xml_doc_out);
    } else {
        printf("no canonical result\n");
    }
    if (wu.error_mask&WU_ERROR_COULDNT_SEND_RESULT) {
        printf("Error: couldn't send a result\n");
    }
    if (wu.error_mask&WU_ERROR_TOO_MANY_ERROR_RESULTS) {
        printf("Error: too many error results\n");
    }
    if (wu.error_mask&WU_ERROR_TOO_MANY_RESULTS) {
        printf("Error: too many total results\n");
    }
}
