#!/usr/bin/env python

## $Id$

# Test whether the scheduling server filters out work units too big for client

from test_uc import *

class WorkTooBig(WorkUC):
    def __init__(self):
        WorkUC.__init__(self)
        self.rsc_disk_bound = 1000000000000   # 1 TB

class ResultUnsent:
    def __init__(self):
        self.server_state = RESULT_SERVER_STATE_UNSENT

class ProjectRsc(ProjectUC):
    def __init__(self):
        ProjectUC.__init__(self, short_name='test_rsc', works=[WorkTooBig()])
    def check(self):
        self.check_results(ResultUnsent())

if __name__ == '__main__':
    test_msg("resource filtering for large work units")
    ProjectRsc()
    run_check_all()
