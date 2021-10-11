#!/usr/bin/env python

## $Id$

# This tests whether the client handles multiple projects, and whether CPU
# time is divided correctly between projects The client should do work for
# project 2 5 times faster than for project 1

from test_uc import *

if __name__ == '__main__':
    test_msg("multiple projects with resource share")
    # create two projects with the same host/user
    host = Host()
    user = UserUC()
    for i in range(2):
        ProjectUC(users=[user], hosts=[host], redundancy=5,
                  short_name="test_1sec_%d"%i, resource_share=[1, 5][i])
    run_check_all()
