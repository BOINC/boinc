#!/usr/bin/env python

## $Id$

# End to end test.  Tests make_work, feeder, scheduling server, client,
# file_upload_handler, validator, assimilator, transitioner, and file_deleter
# on a large batch of workunits.  Confirms that credit is correctly granted
# and that unneeded files are deleted

from test_uc import *
import time, os

class ProjectBackend(ProjectUC):
    def __init__(self, num_wu, redundancy)
        self.num_wu = num_wu
        ProjectUC.__init__(self, redundancy = redundancy, short_name = 'test_backend')
    def make_work(self):
        self.install()
        self.sched_install('make_work', cushion=self.num_wu-1)
        self.start_servers()

        # wait for 500 results to be generated
        n = 0
        db = self.db_open()
        while n < self.num_wu:
            n = num_wus_left(db)
            verbose_echo(1, "Generating workunits [%d/%d]" % (n,self.num_wu));
            time.sleep(.1)
        db.close()
        verbose_echo(1, "Generating workunits [%d/%d]: done." % (self.num_wu,self.num_wu));

        # Stop the project, deinstall make_work, and install the normal
        # backend components
        self.stop()
        self.sched_uninstall('make_work')

    def run(self):
        self.make_work()

        self.sched_install('assimilator')
        self.sched_install('file_deleter')
        self.sched_install('validate_test')
        self.sched_install('feeder')
        self.sched_install('transitioner')
        self.start_servers()

    def check(self):
        # Give the server 20 seconds to finish assimilating/deleting
        # TODO: use wait on all processes.
        verbose_sleep("Sleeping to allow server daemons to finish", 20)
        self.check_results(ResultUC(), self.num)

if __name__ == '__main__':
    num_wu = sys.argv[1:] and get_int(sys.argv[1]) or 100
    redundancy = sys.argv[2:] and get_int(sys.argv[2]) or 5
    test_msg("entire backend (with %d workunits * %d results each)" % num_wu, redundancy);
    ProjectBackend(num_wu = num_wu, redundancy = redundancy)
    run_check_all()
