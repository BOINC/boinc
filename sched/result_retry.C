// result_retry - create new results to make up for lost ones
//
// result_retry
//   [ -dwu n ]
//   [ -dresult n ]
//   [ -nerror n ]
//   [ -ndet n ]
//   [ -nredundancy n ]

void main_loop() {
    WORKUNIT wu;
    RESULT result;
    int nerrors, ndone;

    wu.retry_check_time = time(0);

    // loop over WUs that are due to be checked
    //
    while (db_workunit_enum_check_time(wu)) {
        vector<RESULT> results;

        // enumerate all the results for the WU
        //
        result.workunitid = wu.id;
        while (db_result_enum_workunitid(result)) {
            results.push_back(result);
        }

        nerrors = 0;
        ndone = 0;
        for (i=0; i<results.size(); i++) {
            result = result[i];

            // if any result is unsent, give up on the WU
            //
            if (result.state == RESULT_STATE_UNSENT) {
                fprintf(stderr, "WU %s has unsent result\n", wu.name);
                wu.state = WU_STATE_SEND_FAIL;
                db_workunit_update(wu);
                goto next_wu;
            }
            if (result.state == RESULT_STATE_ERROR) {
                nerrors++;
            }
            if (result.state == RESULT_STATE_DONE) {
                ndone++;
            }
        }

        // it too many errors or too many different results, bail
        //
        if (nerrors > max_errors) {
            fprintf(stderr, "WU %s has too many errors\n", wu.name);
            wu.state = WU_STATE_TOO_MANY_ERRORS;
            db_workunit_update(wu);
            go next_wu;
        }
        if (ndone > max_done) {
            fprintf(stderr, "WU %s has too many answers\n", wu.name);
            wu.state = WU_STATE_TOO_MANY_DONE;
            db_workunit_update(wu);
            go next_wu;
        }

        // generate new results if needed
        //
        n = nredundancy - ndone;
        for (i=0; i<n; i++) {
            create_result(
                wu, result_template_file, suffix, key,
                config.upload_url, config.download_url
            );
        }
next_wu:
    }
}

int main(int argc, char** argv) {
}
