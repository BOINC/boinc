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
        self.sched_install('make_work', cushion=self.num-1)
        self.start_servers()

        # wait for 500 results to be generated
        n = 0
        db = self.db_open()
        while n < self.num:
            n = num_wus_left(db)
            verbose_echo(1, "Generating results [%d/%d]" % (n,self.num));
            time.sleep(.1)
        db.close()
        verbose_echo(1, "Generating results [%d/%d]: done." % (self.num,self.num));

        # Stop the project, deinstall make_work, and install the normal
        # backend components
        self.stop()
        self.sched_uninstall('make_work')
        self.sched_install('assimilator')
        self.sched_install('file_deleter')
        self.sched_install('validate_test')
        self.sched_install('feeder')
        # self.sched_install('timeout_check', nredundancy=0)
        self.start_servers()

    def check(self):
        # Give the server 30 seconds to finish assimilating/deleting
        # TODO: use wait on all processes.
        verbose_sleep("Sleeping to allow server daemons to finish", 30)
        self.check_results(ResultUC(), self.num)

if __name__ == '__main__':
    num = sys.argv[1:] and get_int(sys.argv[1]) or 100
    test_msg("entire backend (with %d results)" % num);
    ProjectBackend(num)
    run_check_all()
