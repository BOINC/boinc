#!/usr/bin/env python

## $Id$

# tests whether command-line arg passing works

from testbase import *

class WorkConcat(Work):
    def __init__(self, redundancy, **kwargs):
        Work.__init__(self, redundancy=redundancy)
        self.wu_template = "concat_wu"
        self.result_template = "concat_result"
        self.input_files = ['input']*2
        self.__dict__.update(kwargs)

class ProjectConcat(TestProject):
    def __init__(self, works=None, users=None, hosts=None):
        (num_wu, redundancy) = get_redundancy_args()
        TestProject.__init__(self,
                             appname = 'concat',
                             num_wu=num_wu, redundancy=redundancy,
                             expected_result = Result(),
                             works = works or [WorkConcat(redundancy=redundancy)],
                             users = users,
                             hosts = hosts)

    # def check(self):
    #     self.sched_run('validate_test')
    #     result = {}
    #     result['server_state'] = RESULT_SERVER_STATE_OVER
    #     self.check_results(result)
    #     self.check_files_match("upload/concat_wu_%d_0", "concat_correct_output", count=self.redundancy)
    #     self.sched_run('assimilator')
    #     self.sched_run('file_deleter')
    #     self.check_deleted("download/input")
    #     self.check_deleted("upload/concat_wu_%d_0", count=self.redundancy)

    # def run(self):
    #     self.install()
    #     self.sched_install('feeder')
    #     self.start_servers()

if __name__ == '__main__':
    test_msg("standard concat application");
    ProjectConcat()
    run_check_all()
