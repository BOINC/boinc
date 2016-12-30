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

# test functions for submit_api.py

from submit_api import *

project_url = 'http://isaac.ssl.berkeley.edu/test/'

# read auth from a file so we don't have to including it here
#
def get_auth():
    with open("test_auth", "r") as f:
        return (f.readline()).strip()

# make a batch description, to be passed to estimate_batch() or submit_batch()
#
def make_batch_desc():
    file = FILE_DESC()
    file.mode = 'remote'
    file.url = 'http://isaac.ssl.berkeley.edu/validate_logic.txt'
    file.md5 = "eec5a142cea5202c9ab2e4575a8aaaa7"
    file.nbytes = 4250

    job = JOB_DESC()
    job.files = [file]

    batch = BATCH_DESC()
    batch.project = project_url
    batch.authenticator = get_auth()
    batch.app_name = "uppercase"
    batch.batch_name = "blah"
    batch.jobs = []

    for i in range(3):
        job.rsc_fpops_est = i*1e9
        job.command_line = '-i %s' %(i)
        batch.jobs.append(copy.copy(job))

    return batch

def test_estimate_batch():
    batch = make_batch_desc()
    #print batch.to_xml("submit")
    r = estimate_batch(batch)
    if r[0].tag == 'error':
        print 'error: ', r.find('error_msg').text
        return
    print 'estimated time: ', r[0].text, ' seconds'

def test_submit_batch():
    batch = make_batch_desc()
    r = submit_batch(batch)
    if r[0].tag == 'error':
        print 'error: ', r.find('error_msg').text
        return
    print 'batch ID: ', r[0].text

def test_query_batches():
    req = REQUEST()
    req.project = project_url
    req.authenticator = get_auth()
    req.get_cpu_time = True
    r = query_batches(req)
    print ET.tostring(r)

def test_query_batch():
    req = REQUEST()
    req.project = project_url
    req.authenticator = get_auth()
    req.batch_id = 271
    req.get_cpu_time = True
    r = query_batch(req)
    if r[0].tag == 'error':
        print 'error: ', r[0].find('error_msg').text
        return
    print ET.tostring(r)
    print 'njobs: ', r.find('njobs').text
    print 'fraction done: ', r.find('fraction_done').text
    print 'total CPU time: ', r.find('total_cpu_time').text
    # ... various other fields
    print 'jobs:'
    for job in r.findall('job'):
        print '   id: ', job.find('id').text
        print '      n_outfiles: ', job.find('n_outfiles').text
        # ... various other fields

def test_abort_batch
    req = REQUEST()
    req.project = project_url
    req.authenticator = get_auth()
    req.batch_id = 271
    r = abort_bath(req)
    if r[0].tag == 'error':
        print 'error: ', r.find('error_msg').text
        return
    print 'success'

def test_upload_files():
    req = UPLOAD_FILES_REQ()
    req.project = project_url
    req.authenticator = get_auth()
    req.batch_id = 271
    req.local_names = ('updater.cpp', 'kill_wu.cpp')
    req.boinc_names = ('xxx_updater.cpp', 'xxx_kill_wu.cpp')
    r = upload_files(req)
    if r[0].tag == 'error':
        print 'error: ', r[0].find('error_msg').text
        return
    print 'success'

test_upload_files()
