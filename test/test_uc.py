#!/usr/bin/env python

## $Id$

# This tests whether the most basic mechanisms are working Also whether stderr
# output is reported correctly Also tests if water levels are working
# correctly

from boinc import *

class ProjectUC(Project):
    def __init__(self):
        Project.__init__(self)
        self.add_core_version()
        self.add_app_and_version("upper_case")
        self.user = User()
        self.user.project_prefs = "<project_specific>\nfoobar\n</project_specific>"
        self.user.global_prefs  = """<venue name=\"home\">
<work_buf_min_days>0</work_buf_min_days>
<work_buf_max_days>2</work_buf_max_days>
<disk_interval>1</disk_interval>
<run_on_batteries/>
<max_bytes_sec_down>400000</max_bytes_sec_down>
</venue>"""
        self.add_user(self.user)
        # must install projects before adding hosts (WHY?)
        self.install()
        self.install_feeder()

        self.host = Host()
        self.host.add_user(self.user, self)
        self.host.install()

        self.work = Work()
        self.work.wu_template = "uc_wu"
        self.work.result_template = "uc_result"
        self.work.redundancy = 2
        self.work.delay_bound = 10

        # Say that 1 WU takes 1 day on a ref comp
        self.work.rsc_fpops = 86400*1e9/2
        self.work.rsc_iops = 86400*1e9/2
        self.work.rsc_disk = 10e8
        self.work.input_files.append('input')
        self.work.install(self)

    def start_servers_and_host(self):
        self.start_servers()
        self.host.run("-exit_when_idle -skip_cpu_benchmarks")

    def validate_all_and_stop(self):
        self.validate(self.work.redundancy)
        result = {}
        result['server_state'] = RESULT_SERVER_STATE_OVER
        result['stderr_out'] = STARTS_WITH("""APP: upper_case: starting, argc 1
APP: upper_case: argv[0] is upper_case
APP: upper_case ending, wrote """)
        # result['exit_status'] = 0
        self.check_results(2, result)
        self.check_files_match("upload/uc_wu_%d_0", "uc_correct_output", count=2)
        self.assimilate()
        self.file_delete()
        self.check_deleted("download/input")
        self.check_deleted("upload/uc_wu_%d_0", count=2)
        self.stop()

if __name__ == '__main__':
    test_msg("standard upper_case application");

    project = ProjectUC()
    project.start_servers_and_host()
    project.validate_all_and_stop()
