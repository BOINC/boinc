#!/usr/bin/env python

## $Id$

# This tests whether the most basic mechanisms are working Also whether stderr
# output is reported correctly Also tests if water levels are working
# correctly

from testbase import *

class UserUC(User):
    def __init__(self):
        User.__init__(self)
        self.project_prefs = "<project_specific>\nfoobar\n</project_specific>"
        self.global_prefs  = """<venue name=\"home\">
<work_buf_min_days>0</work_buf_min_days>
<work_buf_max_days>2</work_buf_max_days>
<disk_interval>1</disk_interval>
<run_on_batteries/>
<max_bytes_sec_down>400000</max_bytes_sec_down>
</venue>"""

class WorkUC(Work):
    def __init__(self, redundancy=2):
        Work.__init__(self)
        self.wu_template = "uc_wu"
        self.result_template = "uc_result"
        self.redundancy = redundancy
        self.delay_bound = 5*redundancy
        self.input_files = ['input']
        # # Say that 1 WU takes 1 day on a ref comp
        #    - note: for test_1sec these values are too high so if you want to
        #            add these back, make them smaller of make test_1sec
        #            request more work
        # self.rsc_fpops = 86400*1e9/2
        # self.rsc_iops = 86400*1e9/2
        # self.rsc_disk = 10e8

class ResultUC:
    def __init__(self):
        self.server_state = RESULT_SERVER_STATE_OVER
        self.client_state = RESULT_FILES_UPLOADED
        self.outcome      = RESULT_OUTCOME_SUCCESS
        self.stderr_out   = MATCH_REGEXPS([ """<stderr_txt>
APP: upper_case: starting, argc \\d+
APP: upper_case: argv[[]0[]] is upper_case
APP: upper_case ending, wrote \\d+ chars"""])
        # self.exit_status = 0

class ResultUCError:
    def __init__(self):
        self.server_state = RESULT_SERVER_STATE_OVER
        self.client_state = RESULT_COMPUTE_DONE
        self.outcome      = RESULT_OUTCOME_CLIENT_ERROR
        self.stderr_out   = MATCH_REGEXPS([ """<stderr_txt>
APP: upper_case: starting, argc \\d+"""])

class ProjectUC(TestProject):
    def __init__(self, works=None, users=None, hosts=None,
                 short_name=None, long_name=None,
                 redundancy=2, resource_share=1):
        TestProject.__init__(self,
                             appname = 'upper_case',
                             works = works or [WorkUC(redundancy=redundancy)],
                             users = users or [UserUC()],
                             hosts = hosts,
                             short_name=short_name, long_name=long_name,
                             redundancy=redundancy, resource_share=resource_share
                             )

    def check(self, result=ResultUC()):
        '''Check results uploaded correctly'''
        self.sched_run('validate_test')
        self.check_results(result)
        self.check_files_match("upload/uc_wu_%d_0", "uc_correct_output", count=self.redundancy)
        self.sched_run('assimilator')
        self.sched_run('file_deleter')
        self.check_deleted("download/input")
        self.check_deleted("upload/uc_wu_%d_0", count=self.redundancy)

    def check_client_error(self, result=ResultUCError()):
        '''Check no results uploaded'''
        self.check_deleted("upload/uc_wu_%d_0", count=self.redundancy)
        self.sched_run('validate_test')
        self.check_results(result)
        self.sched_run('assimilator')
        self.sched_run('file_deleter')

    def run(self):
        self.install()
        self.sched_install('feeder')
        self.start_servers()

if __name__ == '__main__':
    test_msg("standard upper_case application");
    ProjectUC()
    run_check_all()
