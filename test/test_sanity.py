#!/usr/bin/env python

## $Id$

import urllib, random
from boinc import *

# test makes sure that testing framework is sane

def read_url(url):
    '''return 1 line from url'''
    try:
        return urllib.URLopener().open(url).readline().strip()
    except IOError, e:
        error("couldn't access HTML_URL: %s %s" % (HTML_URL, e))
        return ''

if __name__ == '__main__':
    test_msg("framework sanity");

    verbose_echo(1, "Checking executables")
    check_core_client_executable()
    check_app_executable("upper_case")
    check_app_executable("concat")
    check_app_executable("1sec")

    verbose_echo(1, "Checking directories")
    for d in ['KEY_DIR', 'PROJECTS_DIR',
              'CGI_DIR', 'HTML_DIR', 'HOSTS_DIR']:
        dir = globals()[d]
        if not os.path.isdir(dir):
            error("%s doesn't exist: %s" % (d, dir))

    magic = "Foo %x Bar" % random.randint(0,2**16)

    html_path = os.path.join(HTML_DIR, 'test_sanity.txt')
    html_url  = os.path.join(HTML_URL, 'test_sanity.txt')
    cgi_path  = os.path.join(CGI_DIR,  'test_sanity_cgi')
    cgi_url   = os.path.join(CGI_URL,  'test_sanity_cgi')

    verbose_echo(1, "Checking webserver setup: non-cgi")
    print >>open(html_path,'w'), magic
    if read_url(html_url) != magic:
        error("couldn't access a file I just wrote: "+html_path+"\n  using url: "+html_url)

    os.unlink(html_path)

    verbose_echo(1, "Checking webserver setup: cgi")
    print >>open(cgi_path,'w'), '''#!/bin/sh
echo "Content-Type: text/plain"
echo ""
echo "%s"
''' % magic
    os.chmod(cgi_path, 0755)

    if read_url(cgi_url) != magic:
        error("couldn't access a cgi file I just wrote: "+cgi_path+"\n  using url: "+cgi_url)

    os.unlink(cgi_path)
