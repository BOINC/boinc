#!/usr/bin/env python

## $Id$

# End to end test.  Tests make_work, feeder, scheduling server, client,
# file_upload_handler, validator, assimilator, transitioner, and file_deleter
# on a large batch of workunits.  Confirms that credit is correctly granted
# and that unneeded files are deleted

from test_uc import *
import time, os

class ProjectBackend(ProjectUC):
    def __init__(self, num_wu, redundancy):
        self.num_wu = num_wu
        ProjectUC.__init__(self, redundancy = redundancy, short_name = 'test_backend')

    def run(self):
        self.install()

        self.sched_install('make_work', max_wus = self.num_wu)
        self.sched_install('assimilator')
        self.sched_install('file_deleter')
        self.sched_install('validate_test')
        self.sched_install('feeder')
        self.sched_install('transitioner')
        self.start_servers()

        db = self.db_open()
        while True:
            wus_assimilated = num_wus_assimilated(db)
            verbose_echo(1, "Running: WUs [%dassim/%dtotal/%dtarget]  Results: [%dunsent,%dinProg,%dover/%d]" % (
                wus_assimilated, num_wus(db), num_wu,
                num_results_unsent(db),
                num_results_in_progress(db),
                num_results_over(db),
                num_results(db)))
            if wus_assimilated >= num_wu:
                break
            time.sleep(.1)
        db.close()

    def check(self):
        self.check_results(ResultUC(), self.num_wu*redundancy)

if __name__ == '__main__':
    num_wu = sys.argv[1:] and get_int(sys.argv[1]) or 100
    redundancy = sys.argv[2:] and get_int(sys.argv[2]) or 5
    test_msg("entire backend (with %d workunits * %d results each)" % (num_wu, redundancy));
    ProjectBackend(num_wu = num_wu, redundancy = redundancy)
    run_check_all()
