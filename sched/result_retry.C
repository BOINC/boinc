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

    wu.retry_check_time = time(0);
    while (db_workunit_enum_check_time(wu)) {
        vector<RESULT> results;
        result.workunitid = wu.id;
        while (db_result_enum_workunitid(result)) {
            results.push_back(result);
        }
    }
}

int main(int argc, char** argv) {
}
