#!/usr/bin/env python

## $Id$

# Make sure server hears that client exited with nonzero status.

from test_uc import *

class WorkExit(WorkUC):
    def __init__(self):
        WorkUC.__init__(self)
        self.wu_template = "uc_exit_wu"

class ResultExit(ResultUCError):
    def __init__(self):
        ResultUCError.__init__(self)
        self.stderr_out.append('<message>process exited with a non-zero exit code')

class ProjectExit(ProjectUC):
    def __init__(self):
        ProjectUC.__init__(self, short_name='test_exit', works=[WorkExit()])
    def check(self):
        self.check_client_error(ResultExit())

if __name__ == '__main__':
    test_msg("application exit report mechanism")
    ProjectExit()
    run_check_all()
