#!/usr/bin/env python

## $Id$

# DOESN'T WORK YET

from test_uc import *

class ProjectSchedMove(ProjectUC):
    def run(self):
        self.sched_install('feeder')
        self.start_servers()


if __name__ == '__main__':
    test_msg("backing off and finding new scheduler URL from master URL")
    verbose_echo(0, "BEFORE SCHEDULER MOVE")
    proj = ProjectSchedMove(short_name='test_sched_moved')
    proj.install()
    run_check_all()

    verbose_echo(0, "AFTER SCHEDULER MOVE")
    # TODO: change scheduler url
    proj.install_project()
    proj.install_works()
    run_check_all()

