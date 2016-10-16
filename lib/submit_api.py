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


# Python bindings of remote job submission and file management APIs

import urllib
import copy
import xml.etree.ElementTree as ET
import requests
    # you'll need to "yip install requests"

# represents an input file
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

# represents a job
#
class JOB_DESC:
    def __init__(self):
        return
    def to_xml(self):
        xml = ('<job>\n'
        '<rsc_fpops_est>%f</rsc_fpops_est>\n'
        '<command_line>%s</command_line>\n'
        ) %(self.rsc_fpops_est, self.command_line)
        for file in self.files:
            xml += file.to_xml()
        xml += '</job>\n'
        return xml

# represents a batch description for submit() or estimate()
#
class BATCH_DESC:
    def __init__(self):
        return

    def to_xml(self, op):
        xml = ('<%s>\n'
        '<authenticator>%s</authenticator>\n'
        '<batch>\n'
        '<app_name>%s</app_name>\n'
        '<batch_name>%s</batch_name>\n'
        ) %(op, self.authenticator, self.app_name, self.batch_name)
        for job in self.jobs:
            xml += job.to_xml()
        xml += '</batch>\n</%s>\n' %(op)
        return xml

# a generic request
#
class REQUEST:
    def __init__(self):
        return

def do_http_post(req, project_url):
    url = project_url + 'submit_rpc_handler.php'
    params = urllib.urlencode({'request': req})
    f = urllib.urlopen(url, params)
    reply = f.read()
    print reply
    return ET.fromstring(reply)

def estimate_batch(req):
    return do_http_post(req.to_xml('estimate_batch'), req.project)

def submit_batch(req):
    return do_http_post(req.to_xml('submit_batch'), req.project)

def query_batches(req):
    req_xml = ('<query_batches>\n'
    '<authenticator>%s</authenticator>\n'
    '<get_cpu_time>%d</get_cpu_time>\n'
    '</query_batches>\n'
    ) %(req.authenticator, 1 if req.get_cpu_time else 0)
    return do_http_post(req_xml, req.project)

def query_batch(req):
    req_xml = ('<query_batch>\n'
    '<authenticator>%s</authenticator>\n'
    '<batch_id>%s</batch_id>\n'
    '<get_cpu_time>%d</get_cpu_time>\n'
    '</query_batch>\n'
    ) %(req.authenticator, req.batch_id, 1 if req.get_cpu_time else 0)
    return do_http_post(req_xml, req.project)

def query_job(req):
    req_xml = ('<query_job>\n'
    '<authenticator>%s</authenticator>\n'
    '<job_id>%s</job_id>\n'
    '</query_job>\n'
    ) %(req.authenticator, req.job_id)
    return do_http_post(req_xml, req.project)

def abort_batch(req):
    req_xml = ('<abort_batch>\n'
    '<authenticator>%s</authenticator>\n'
    '<batch_id>%s</batch_id>\n'
    '</abort_batch>\n'
    ) %(req.authenticator, req.batch_id)
    return do_http_post(req_xml, req.project)

def get_output_file(req):
    auth_str = md5.new(req.authenticator+req.instance_name).digest()
    name = req.instance_name
    file_num = req.file_num
    return project_url+"/get_output.php?cmd=result_file&result_name=%s&file_num=%s&auth_str=%s"%(name, file_num, auth_str)

def get_output_files(req):
    auth_str = md5.new(req.authenticator+req.batch_id).digest()
    return project_url+"/get_output.php?cmd=batch_files&batch_id=%s&auth_str=%s"%(req.batch_id, auth_str)


def retire_batch(req):
    req_xml = ('<retire_batch>\n'
    '<authenticator>%s</authenticator>\n'
    '<batch_id>%s</batch_id>\n'
    '</retire_batch>\n'
    ) %(req.authenticator, req.batch_id)
    return do_http_post(req_xml, project_url)

############ FILE MANAGEMENT API ##############

class QUERY_FILES_REQ:
    def __init__(self):
        return

    def to_xml(self):
        xml = ('<query_files>\n'
        '<authenticator>%s</authenticator>\n'

class UPLOAD_FILES_REQ:
    def __init__(self):
        return

    def to_xml(self):
        xml = ('<upload_files>\n'
        '<authenticator>%s</authenticator>\n'

def query_files(query_files_req):
    return do_http_post(query_files_req.to_xml(), req.project)

def upload_files(upload_files_req):
    return do_http_post(upload_files_req.to_xml(), req.project)

