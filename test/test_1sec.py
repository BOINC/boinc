#!/usr/bin/env python

## $Id$

# This tests whether the client handles multiple projects, and whether CPU
# time is divided correctly between projects The client should do work for
# project 2 5 times faster than for project 1

from boinc import *
from test_uc import *

if __name__ == '__main__':
    test_msg("multiple projects with resource share");

    host = Host()
    user = UserUC()
    work = WorkUC(redundancy=5)
    projects = []
    for i in range(2):
        project = ProjectUC(users=[user], hosts=[host], works=[work],
                            short_name="test_1sec_%d"%i)
        project.resource_share = [1, 5][i]
        projects.append(project)
        project.run()
    host.run()
    for project in projects:
        project.stop()
        project.validate()
