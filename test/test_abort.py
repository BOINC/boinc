#!/usr/bin/env python

## $Id$

# Makes sure that the client aborts when the output file size limit is
# exceeded, and that the server knows it.

from test_uc import *

class WorkAbort(WorkUC):
    def __init__(self):
        WorkUC.__init__(self)
        self.result_template = "abort_result"

class ResultAbort(ResultUC):
    def __init__(self):
        ResultUC.__init__(self)
        self.client_state = RESULT_OUTCOME_CLIENT_ERROR

class ProjectAbort(ProjectUC):
    def __init__(self):
        ProjectUC.__init__(self, short_name='test_abort', works=[WorkAbort()])

    def check(self):
        # no results should have been uploaded
        self.check_deleted("upload/uc_wu_%d_0", count=self.redundancy)
        self.sched_run('validate_test')
        self.check_results(ResultAbort())
        self.sched_run('assimilator')
        self.sched_run('file_deleter')

if __name__ == '__main__':
    test_msg("result abort mechanism (disk space limit)")
    ProjectAbort()
    run_check_all()
