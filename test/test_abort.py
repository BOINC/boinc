#!/usr/bin/env python

## $Id$

# Makes sure that the client aborts when the output file size limit is
# exceeded, and that the server knows it.

from test_uc import *

class WorkAbort(WorkUC):
    def __init__(self):
        WorkUC.__init__(self)
        self.result_template = "abort_result"

class ResultAbort(ResultUCError):
    def __init__(self):
        ResultUCError.__init__(self)
        self.stderr_out.append('<message>Output file exceeded size limit')

class ProjectAbort(ProjectUC):
    def __init__(self):
        ProjectUC.__init__(self, short_name='test_abort', works=[WorkAbort()])
    def check(self):
        self.check_client_error(ResultAbort())

if __name__ == '__main__':
    test_msg("result abort mechanism (disk space limit)")
    ProjectAbort()
    run_check_all()
