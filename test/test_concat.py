#!/usr/bin/env python

## $Id$

# tests whether command-line arg passing works

from boinc import *

class WorkConcat(Work):
    def __init__(self, redundancy=2):
        Work.__init__(self)
        self.wu_template = "concat_wu"
        self.result_template = "concat_result"
        self.redundancy = redundancy
        self.input_files = ['input']*2

class ProjectConcat(Project):
    def __init__(self, works=None, users=None, hosts=None):
        Project.__init__(self,
                         appname = 'concat',
                         works = works or [WorkConcat()],
                         users = users,
                         hosts = hosts)

    def validate(self):
        redundancy = self.work.redundancy
        Project.validate(self, redundancy)
        result = {}
        result['server_state'] = RESULT_SERVER_STATE_OVER
        self.check_results(redundancy, result)
        self.check_files_match("upload/concat_wu_%d_0", "concat_correct_output", count=redundancy)
        self.assimilate()
        self.file_delete()
        self.check_deleted("download/input")
        self.check_deleted("upload/concat_wu_%d_0", count=redundancy)

    def run(self):
        self.install()
        self.install_feeder()
        self.start_servers()

if __name__ == '__main__':
    test_msg("standard concat application");

    project = ProjectConcat()
    project.run()
    project.host.run()
    project.validate()
    project.stop()
