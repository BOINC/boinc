# This file is part of BOINC.
# http://boinc.berkeley.edu
# Copyright (C) 2016 University of California
#
# BOINC is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
#
# BOINC is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

# test code for submit_api.py

from submit_api import *

# read auth from a file so we don't have to including it here
#
def get_auth():
    with open("test_auth", "r") as f:
        return (f.readline()).strip()

def make_batch():
    file = FILE_DESC()
    file.mode = 'remote'
    file.url = 'http://isaac.ssl.berkeley.edu/validate_logic.txt'
    file.md5 = "eec5a142cea5202c9ab2e4575a8aaaa7"
    file.nbytes = 4250

    job = JOB_DESC()
    job.files = [file]

    batch = BATCH_DESC()
    batch.project = 'http://isaac.ssl.berkeley.edu/test/'
    batch.authenticator = get_auth()
    batch.app_name = "uppercase"
    batch.batch_name = "blah"
    batch.jobs = []

    for i in range(3):
        job.rsc_fpops_est = i*1e9
        job.command_line = '-i %s' %(i)
        batch.jobs.append(copy.copy(job))

    return batch

def test_estimate():
    batch = make_batch()
    #print batch.to_xml("submit")
    r = estimate_batch(batch)
    #print ET.tostring(r)
    if r.tag == 'error':
        print 'error: ', r.find('error_msg').text
    else:
        print 'estimated time: ', r.text, ' seconds'

def test_query_batches():
    req = REQUEST()
    req.project = 'http://isaac.ssl.berkeley.edu/test/'
    req.authenticator = get_auth()
    req.get_cpu_time = True
    r = query_batches(req)
    print ET.tostring(r)

def test_query_batch():
    req = REQUEST()
    req.project = 'http://isaac.ssl.berkeley.edu/test/'
    req.authenticator = get_auth()
    req.batch_id = 101
    req.get_cpu_time = True
    r = query_batch(req)
    print ET.tostring(r)

test_estimate()
