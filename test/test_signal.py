#!/usr/bin/env python

## $Id$

# Make sure server hears that client died by a signal.

from test_uc import *

class WorkSignal(WorkUC):
    def __init__(self):
        WorkUC.__init__(self)
        self.wu_template = "uc_sig_wu"

class ResultSignal(ResultUCError):
    def __init__(self):
        ResultUCError.__init__(self)
        self.stderr_out.append('SIGHUP: terminal line hangup')
        self.stderr_out.append('<message>process exited with a non-zero exit code')

class ProjectSignal(ProjectUC):
    def __init__(self):
        ProjectUC.__init__(self, short_name='test_signal', works=[WorkSignal()])
    def check(self):
        self.check_client_error(ResultSignal())

if __name__ == '__main__':
    test_msg("application signal report mechanism")
    ProjectSignal()
    run_check_all()
