#!/usr/bin/env python

## $Id$

from test_uc import *

if __name__ == '__main__':
    test_msg("scheduler exponential backoff (master url failure)")
    # Proxy('close_connection if $nconnections < -2; '+
    #       'kill 6,%d if $nconnections>=100 '%os.getpid(), html=1)
    # Proxy(('print "hi nc=$nconnections start=$start nchars=$nchars url=$url\n"; ' +
    #        'close_connection if $nconnections < 2; '),
    #       html=1)
    Proxy(( 'print "hi nc=$nconnections start=$start nchars=$nchars url=$url\n"; ' +
           ''),
          html=1)
    ProjectUC()
    run_check_all()

    ## TODO: verify it took ??? seconds
    ## TODO: time out after ??? seconds and fail this test
