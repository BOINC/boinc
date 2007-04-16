#!/usr/bin/env python

# $Id$

# fake php - all we really need is 'include schedulers.txt'

import os, sys

REQUEST_URI = os.environ['REQUEST_URI']

print 'Content-Type: text/plain'
print

print """--- FAKE PHP ---

[ REQUEST_URI=%s ]

Since I can't find php4 on your system, this stub program fake_php.py just
prints schedulers.txt as necessary.

"""%REQUEST_URI

if REQUEST_URI.endswith('/index.php'):
    sys.stdout.write(open('schedulers.txt').read())
