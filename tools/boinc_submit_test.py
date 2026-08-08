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

# test functions for boinc_submit.py

# YOU MUST CREATE A FILE "test_auth' CONTAINING
#
# project URL
# authenticator of your account

from boinc_submit import *

# read URL and auth from a file so we don't have to include it here
#
def get_auth():
    with open("test_auth", "r") as f:
        url = (f.readline()).strip()
        auth = (f.readline()).strip()
    return [url, auth]

[url, auth] = get_auth()
s = BOINC_SERVER(url, auth)

def check_error(r):
    if 'error' in r.keys():
        x = r['error']
        print('error: num: %s msg: %s'%(x['error_num'], x['error_msg']))
        return True
    return False

def test_estimate_batch():
    f = FILE_DESC('main_2.sh', 'sandbox')
    j = JOB_DESC([f]);
    b = BATCH_DESC('uppercase', [j])
    r = s.estimate_batch(b)
    if check_error(r):
        return
    print('estimated time: ', r['seconds'])

def test_submit_batch(batch_name):
    f = FILE_DESC('main_2.sh', 'sandbox')
    j = JOB_DESC([f]);
    b = BATCH_DESC('uppercase', [j])
    r = s.submit_batch(b)
    if check_error(r):
        return
    print('batch ID: ', r['batch_id'])

def test_query_batches():
    r = s.query_batches(True)
    if check_error(r):
        return
    print(r)

def test_query_batch(id):
    r = s.query_batch(id, True, True)
    if check_error(r):
        return
    print(r);
    print('njobs: ', r['njobs']);
    print('fraction_done: ', r['fraction_done']);
    print('total_cpu_time: ', r['total_cpu_time']);
    # ... various other fields
    print('jobs:')
    if 'job' in r.keys():
        x = r['job']
        if not isinstance(x, list):
            x = [x]
        for job in x:
            print('   id: ', job['id'])
            # ... various other fields

def test_create_batch(name):
    r = s.create_batch('uppercase', name, 0)
    if check_error(r):
        return
    print('batch ID: ', r['batch_id'])

def test_abort_batch(id):
    r = s.abort_batch(id)
    if check_error(r):
        return
    print('success')

def test_upload_files():
    batch_id = 283
    local_names = ('updater.cpp', 'kill_wu.cpp')
    phys_names = ('dxxxb_updater.cpp', 'dxxxb_kill_wu.cpp')
    r = s.upload_files(local_names, phys_names, batch_id, 0)
    if check_error(r):
        return
    print('upload_files: success')

def test_query_files():
    batch_id = 271
    phys_names = ['dxxxb_updater.cpp', 'dxxx_kill_wu.cpp']
    r = s.query_files(phys_names, batch_id, 0)
    if check_error(r):
        return
    print('absent files:')
    for f in r['absent_files']['file']:
        print(f)

def test_get_output_file():
    req = REQUEST()
    [req.project, req.authenticator] = get_auth()
    req.instance_name = 'uppercase_32275_1484961754.784017_0_0'
    req.file_num = 1
    r = get_output_file(req)
    print(r)

def test_get_output_files():
    req = REQUEST()
    [req.project, req.authenticator] = get_auth()
    req.batch_id = 271
    r = get_output_files(req)
    print(r)

def test_get_job_counts():
    req = REQUEST()
    [req.project, req.authenticator] = get_auth()
    x = get_job_counts(req)
    print(x.find('results_ready_to_send').text)

#test_query_batch(520)
#test_abort_batch(117)
#test_query_batches()
#test_submit_batch('batch_39')
#test_estimate_batch()
test_upload_files()
#test_query_files()
#test_create_batch('batch_140')
