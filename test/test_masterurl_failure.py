#!/usr/bin/env python

## $Id$

from test_uc import *

if __name__ == '__main__':
    test_msg("scheduler exponential backoff (master url failure)")
    Proxy('close_connection if $nconnections < 2; ', html=1)
    # Proxy(('print "hi nc=$nconnections start=$start nchars=$nchars url=$url\n"; ' +
    #        'close_connection if $nconnections < 2; '),
    #       html=1)
    # Proxy(( 'print "hi nc=$nconnections start=$start nchars=$nchars url=$url\n"; ' +
    #        ''),
    #       html=1)
    ProjectUC(short_name='test_masterurl_failure')
    run_check_all()

    ## TODO: verify it took ??? seconds
