# THIS IS DEPRECATED.  USE boinc_submit.py INSTEAD

# This file is part of BOINC.
# http://boinc.berkeley.edu
# Copyright (C) 2020 University of California
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


# Python bindings of the remote job submission and file management APIs
# See https://github.com/BOINC/boinc/wiki/RemoteJobs#Pythonbinding

import urllib
import urllib2
import copy
import xml.etree.ElementTree as ET
import requests
    # you'll need to "pip install requests"
import hashlib

# describes an input file
#
class FILE_DESC:
    def __init__(self):
        return
    def to_xml(self):
        xml = ('<input_file>\n'
        '<mode>%s</mode>\n'
        ) %(self.mode)
        if self.mode == 'remote':
            xml += ('<url>%s</url>\n'
            '<nbytes>%f</nbytes>\n'
            '<md5>%s</md5>\n'
            ) %(self.url, self.nbytes, self.md5)
        else:
            xml += '<source>%s</source>\n' %(self.source)
        xml += '</input_file>\n'
        return xml

# describes a job
#
class JOB_DESC:
    def __init__(self):
        return
    def to_xml(self):
        xml = '<job>\n'
        if hasattr(self, 'name'):
            xml += '<name>%s</name>\n'%self.name
        if hasattr(self, 'rsc_fpops_est'):
            xml += '<rsc_fpops_est>%f</rsc_fpops_est>\n'%self.rsc_fpops_est
        if hasattr(self, 'command_line'):
            xml += '<command_line>%s</command_line>\n'%self.command_line
        if hasattr(self, 'input_template'):
            xml += '%s\n'%self.input_template
        if hasattr(self, 'output_template'):
            xml += '%s\n'%self.output_template
        if hasattr(self, 'priority'):
            xml += '<priority>%d</priority>\n'%(self.priority)
        if hasattr(self, 'files'):
            for file in self.files:
                xml += file.to_xml()
        xml += '</job>\n'
        return xml

# describes a batch for submit() or estimate()
#
class BATCH_DESC:
    def __init__(self):
        return

    def to_xml(self, op):
        xml = ('<%s>\n'
            '<authenticator>%s</authenticator>\n'
            '<batch>\n'
            '<app_name>%s</app_name>\n'
        ) %(op, self.authenticator, self.app_name)

        if hasattr(self, 'batch_id'):
            xml += '<batch_id>%s</batch_id>\n'%(self.batch_id)
        elif hasattr(self, 'batch_name'):
            xml += '<batch_name>%s</batch_name>\n'%(self.batch_name)

        if hasattr(self, 'app_version_num'):
            xml += '<app_version_num>%d</app_version_num>\n'%(self.app_version_num)

        if hasattr(self, 'allocation_priority'):
            if self.allocation_priority:
                xml += '<allocation_priority/>\n'
        if hasattr(self, 'priority'):
            xml += '<priority>%d</priority>\n'%(self.priority)
        for job in self.jobs:
            xml += job.to_xml()
        xml += '</batch>\n</%s>\n' %(op)
        return xml

class CREATE_BATCH_REQ:
    def __init__(self):
        return
    def to_xml(self):
        xml = ('<create_batch>\n'
        '<authenticator>%s</authenticator>\n'
        '<app_name>%s</app_name>\n'
        '<batch_name>%s</batch_name>\n'
        '<expire_time>%f</expire_time>\n'
        '</create_batch>\n') %(self.authenticator, self.app_name, self.batch_name, self.expire_time)
        return xml

# a generic request
#
class REQUEST:
    def __init__(self):
        return

rpc_timeout = 0

def do_http_post(req, project_url, handler='submit_rpc_handler.php'):
    #print(req)
    url = project_url + handler
    params = urllib.urlencode({'request': req})
    if rpc_timeout>0:
        f = urllib2.urlopen(url, params, rpc_timeout)
    else:
        f = urllib2.urlopen(url, params)

    reply = f.read()
    #print("REPLY:", reply)
    return ET.fromstring(reply)

########### API FUNCTIONS START HERE ###############

def set_timeout(x):
    global rpc_timeout
    rpc_timeout = x

def abort_batch(req):
    req_xml = ('<abort_batch>\n'
    '<authenticator>%s</authenticator>\n'
    '<batch_id>%s</batch_id>\n'
    '</abort_batch>\n'
    ) %(req.authenticator, req.batch_id)
    return do_http_post(req_xml, req.project)

def abort_jobs(req):
    req_xml = ('<abort_jobs>\n'
    '<authenticator>%s</authenticator>\n'
    ) %(req.authenticator)
    for job in req.jobs:
        req_xml += '<job_name>%s</job_name>\n'%(job)
    req_xml += '</abort_jobs>\n'
    return do_http_post(req_xml, req.project)

# req is a CREATE_BATCH_REQ
#
def create_batch(req):
    return do_http_post(req.to_xml(), req.project)

def estimate_batch(req):
    return do_http_post(req.to_xml('estimate_batch'), req.project)

def query_batch(req):
    req_xml = ('<query_batch>\n'
    '<authenticator>%s</authenticator>\n'
    '<batch_id>%s</batch_id>\n'
    '<get_cpu_time>%d</get_cpu_time>\n'
    '<get_job_details>%d</get_job_details>\n'
    '</query_batch>\n'
    ) %(req.authenticator, req.batch_id, 1 if req.get_cpu_time else 0, 1 if req.get_job_details else 0)
    return do_http_post(req_xml, req.project)

def query_batches(req):
    req_xml = ('<query_batches>\n'
    '<authenticator>%s</authenticator>\n'
    '<get_cpu_time>%d</get_cpu_time>\n'
    '</query_batches>\n'
    ) %(req.authenticator, 1 if req.get_cpu_time else 0)
    return do_http_post(req_xml, req.project)

def query_completed_job(req):
    req_xml = ('<query_completed_job>\n'
    '<authenticator>%s</authenticator>\n'
    '<job_name>%s</job_name>\n'
    '</query_completed_job>\n'
    ) %(req.authenticator, req.job_name)
    return do_http_post(req_xml, req.project)

def query_job(req):
    req_xml = ('<query_job>\n'
    '<authenticator>%s</authenticator>\n'
    '<job_id>%s</job_id>\n'
    '</query_job>\n'
    ) %(req.authenticator, req.job_id)
    return do_http_post(req_xml, req.project)

def get_output_file(req):
    auth_str = hashlib.md5(req.authenticator+req.instance_name).hexdigest()
    name = req.instance_name
    file_num = req.file_num
    return req.project+"/get_output.php?cmd=result_file&result_name=%s&file_num=%s&auth_str=%s"%(name, file_num, auth_str)

def get_output_files(req):
    auth_str = hashlib.md5(req.authenticator+str(req.batch_id)).hexdigest()
    return req.project+"/get_output.php?cmd=batch_files&batch_id=%s&auth_str=%s"%(req.batch_id, auth_str)

def retire_batch(req):
    req_xml = ('<retire_batch>\n'
    '<authenticator>%s</authenticator>\n'
    '<batch_id>%s</batch_id>\n'
    '</retire_batch>\n'
    ) %(req.authenticator, req.batch_id)
    return do_http_post(req_xml, req.project)

def submit_batch(req):
    return do_http_post(req.to_xml('submit_batch'), req.project)

# see if reply is error.
# if so print the message and return True
#
def check_error(response):
    if response.find('error') is not None:
         print('BOINC server error: ', response.find('error').find('error_msg').text)
         return True

############ FILE MANAGEMENT API ##############

class QUERY_FILES_REQ:
    def __init__(self):
        return

    def to_xml(self):
        xml = ('<query_files>\n'
        '<authenticator>%s</authenticator>\n'
        '<batch_id>%d</batch_id>\n') %(self.authenticator, self.batch_id)
        for name in self.boinc_names:
            xml += '<phys_name>%s</phys_name>\n' %(name)
        xml += '</query_files>\n'
        return xml

class UPLOAD_FILES_REQ:
    def __init__(self):
        return

    def to_xml(self):
        xml = ('<upload_files>\n'
        '<authenticator>%s</authenticator>\n'
        '<batch_id>%d</batch_id>\n') %(self.authenticator, self.batch_id)
        for name in self.boinc_names:
            xml += '<phys_name>%s</phys_name>\n' %(name)
        xml += '</upload_files>\n'
        return xml

def query_files(query_req):
    reply = do_http_post(query_req.to_xml(), query_req.project, 'job_file.php')
    return reply

# This actually does two RPCs:
# query_files() to find what files aren't already on server
# upload_files() to upload them
#
def upload_files(upload_files_req):
    query_req = QUERY_FILES_REQ()
    query_req.authenticator = upload_files_req.authenticator
    query_req.batch_id = upload_files_req.batch_id
    query_req.boinc_names = upload_files_req.boinc_names
    query_req_xml = query_req.to_xml()
    reply = do_http_post(query_req_xml, upload_files_req.project, 'job_file.php')
    if reply[0].tag == 'error':
        return reply

    absent = reply.find('absent_files').findall('file')
    #print('query files succeeded; ',len(absent), ' files need upload')
    boinc_names = []
    local_names = []
    for n in absent:
        ind = int(n.text)
        boinc_names.append(upload_files_req.boinc_names[ind])
        local_names.append(upload_files_req.local_names[ind])
    upload_files_req.boinc_names = boinc_names
    upload_files_req.local_names = local_names

    # make a description of upload files for "requests"
    #
    files = []
    for i in range(len(boinc_names)):
        bn = boinc_names[i]
        ln = local_names[i]
        upload_name = 'file_%d'%(i)
        files.append((upload_name, (bn, open(ln, 'rb'), 'application/octet-stream')))

    url = upload_files_req.project + '/job_file.php'
    req_xml = upload_files_req.to_xml()
    #print(req_xml)
    req = {'request': req_xml}
    reply = requests.post(url, data=req, files=files)
    #print("reply text: ", reply.text)
    return ET.fromstring(reply.text)

# returns an XML object with various job counts
#   results_ready_to_send
#   results_in_progress
#   results_need_file_delete
#   wus_need_validate
#   wus_need_assimilate
#   wus_need_file_delete
# see tools/submit_api_test.py
#
def get_job_counts(req):
    return do_http_post('', req.project, 'server_status.php?counts=1')
