#!/usr/bin/env python

## $Id$

# End to end test.  Tests make_work, feeder, scheduling server, client,
# file_upload_handler, validator, assimilator, timeout_check, and file_deleter
# on a large batch of workunits.  Confirms that credit is correctly granted
# and that unneeded files are deleted

from boinc import *
from test_uc import *
import time, os

class ProjectBackend(ProjectUC):
    def __init__(self, num):
        self.num = num
        ProjectUC.__init__(self, redundancy = 5, short_name = 'test_backend')
    def run(self):
        self.install()

        self.install_make_work(work=self.work, cushion=self.num-1, redundancy=5)
        self.start_servers()

        # wait for 500 results to be generated
        n = 0
        db = self.db_open()
        while n < self.num:
            n = self.num_wus_left(db)
            verbose_echo(1, "Generating results [%d/%d]" % (n,self.num));
            time.sleep(.1)
        db.close()
        verbose_echo(1, "Generating results [%d/%d]: done." % (self.num,self.num));

        # Stop the project, deinstall make_work, and install the normal
        # backend components
        self.stop()
        self.uninstall_make_work()
        self.install_assimilator()
        self.install_file_delete()
        self.install_validate(self.app, quorum=5)
        self.install_feeder()
        # self.install_timeout_check(self.app, nerror=5, ndet=5, nredundancy=0)

        # TODO: get PID and use wait.
        verbose_echo(1, "Waiting for make_work to finish...")
        while not os.system('pgrep -n make_work >/dev/null'):
            time.sleep(1)
        self.start_servers()

    rpm_pid = None
    def fork_result_progress_meter(self, msg):
        pid = os.fork()
        if pid:
            self.rpm_pid = pid
            return
        db = self.db_open()
        while True:
            n = self.num_results_done(db)
            verbose_echo(1, msg+" [%d/%d]"%(n,self.num))
            time.sleep(.1)

    def stop_result_progress_meter(self):
        if self.rpm_pid:
            os.kill(self.rpm_pid, 9)

    def check(self):
        # Give the server 30 seconds to finish assimilating/deleting
        # TODO: use wait on all processes.
        verbose_sleep("Sleeping to allow server to finish", 30)
        self.check_results(self.num, ResultUC())

if __name__ == '__main__':
    test_msg("entire backend");

    num = 100
    try: num = int(sys.argv[1])
    except: pass

    project = ProjectBackend(num)
    project.run()
    project.fork_result_progress_meter("Running core client - results done: ")
    project.host.run()
    project.stop_result_progress_meter()
    project.check()
    project.stop()
