#!/usr/bin/env python

## $Id$

from test_uc import *

if __name__ == '__main__':
    test_msg("scheduler exponential backoff (master url failure)")
    proxy = Proxy('exit 1 if $nconnections < 4; if_done_kill(); if_done_ping();',
                  html=1)
    ProjectUC()
    run_check_all()

    ## TODO: verify it took ??? seconds
    ## TODO: time out after ??? seconds and fail this test
