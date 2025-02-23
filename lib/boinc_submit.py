# This file is part of BOINC.
# https://boinc.berkeley.edu
# Copyright (C) 2024 University of California
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


# Python binding for remote job submission and file management RPCs

import urllib, copy, requests, hashlib
import xml.etree.ElementTree as ElementTree

# a job input file.
# modes:
#   local_staged
#       file is already staged.  source is physical name
#   local
#       source is full path on server
#       on job submission, file will be staged and a physical name assigned
#   semilocal
#       source is URL
#       on job submission, file will be fetched and staged,
#       and a physical name assigned
#   remote
#       source is URL; nbytes and MD5 must be supplied
#       file will be fetched by client
#   sandbox
#       source is name in sandbox; file is already staged
#   inline
#       source is file contents; name will be assigned
#
class FILE_DESC:
    def __init__(
        self, source=None, mode='local_staged', nbytes=None, md5=None
    ):
        self.source = source
        self.mode = mode
        self.nbytes = nbytes
        self.md5 = md5

# description of a job
#
class JOB_DESC:
    def __init__(self, files=[], name=None, rsc_fpops_est=None,
        command_line=None, input_template=None, output_template=None,
        priority=None
    ):
        self.files = files
        self.name = name
        self.rsc_fpops_est = rsc_fpops_est
        self.command_line = command_line
        self.input_template = input_template
        self.output_template = output_template
        self.priority = priority

# description of a batch
#
class BATCH_DESC:
    def __init__(
        self, app_name, jobs, batch_id=0, batch_name=None,
        expire_time=None, app_version_num=0,
        priority=None, allocation_priority=False
    ):
        self.app_name = app_name
        self.jobs = jobs
        self.batch_name = batch_name
        self.batch_id = batch_id
        self.expire_time = expire_time
        self.app_version_num = app_version_num
        self.priority = priority
        self.allocation_priority = allocation_priority

class BOINC_SERVER:
    def __init__(self, url, authenticator, rpc_timeout=0):
        self.url = url
        self.authenticator = authenticator
        self.rpc_timeout = rpc_timeout

    # All operations return a Python dictionary

    # batch operations

    def submit_batch(self, batch_desc):
        return self.do_http_post(self.batch_desc_xml(batch_desc, 'submit_batch'))

    def create_batch(self, app_name, batch_name='', expire_time=0):
        return self.do_http_post(
            self.create_batch_xml(app_name, batch_name, expire_time)
        )

    def estimate_batch(self, batch_desc):
        return self.do_http_post(self.batch_desc_xml(batch_desc, 'estimate_batch'))

    def abort_batch(self, batch_id):
        return self.do_http_post(self.batch_op_xml(batch_id, 'abort_batch'))

    def retire_batch(self, batch_id):
        return self.do_http_post(self.batch_op_xml(batch_id, 'retire_batch'))

    def query_batch(self, batch_id, get_cpu_time=False, get_job_details=False):
        return self.do_http_post(
            self.query_batch_xml(batch_id, get_cpu_time, get_job_details)
        )

    def query_batches(self, get_cpu_time):
        return self.do_http_post(self.query_batches_xml(get_cpu_time))

########## job operations

    def query_completed_job(self, job_name):
        return self.do_http_post(self.query_completed_job_xml(job_name))

    def abort_jobs(self, job_names):
        return self.do_http_post(self.abort_jobs_xml(job_names))

    def get_job_counts(self):
        return self.do_http_post('', 'server_status.php?counts=1')

########## file operations

    # returns list of files missing on server.
    # updates delete time and batch association of ones that are present
    #
    def query_files(self, phys_names, batch_id=0, delete_time=0):
        return self.do_http_post(
            self.query_files_xml(phys_names, batch_id, delete_time),
            'job_file.php'
        )

    # this calls query_files() first and uploads only the missing files
    #
    def upload_files(self, local_names, phys_names, batch_id=0, delete_time=0):
        ret = self.do_http_post(
            self.query_files_xml(phys_names, batch_id, delete_time),
            'job_file.php'
        )
        if 'error' in ret:
            return ret
        absent = ret['absent_files']
        if isinstance(absent, str):
            return {'success':None}
        if 'file' not in absent.keys():
            return {'success':None}
        file = absent['file']
        if type(file) != list:
            file = [file]
        n = 0
        files = []
        upload_phys_names = []
        for f in file:
            i = int(f)
            pn = phys_names[i]
            ln = local_names[i]
            upload_name = 'file_%d'%(n)
            upload_phys_names.append(pn)
            files.append((upload_name, (pn, open(ln, 'rb'),'application/octet-stream')))
            n += 1
        url = self.url + 'job_file.php'
        req_xml = self.upload_files_xml(upload_phys_names, batch_id, delete_time)
        req = {'request': req_xml}
        reply = requests.post(url, data=req, files=files)
        root = ElementTree.XML(reply.text)
        return etree_to_dict(root)

    # return URL for fetching a particular output file
    # The result must belong to a batch owned by this user.
    # Prevents a submitter from seeing results from another submitter
    #
    def get_output_file_url(self, result_name, file_num):
        auth_str = hashlib.md5(bytes(self.authenticator+result_name, 'utf-8')).hexdigest()
        return self.url+"get_output.php?cmd=result_file&result_name=%s&file_num=%s&auth_str=%s"%(result_name, file_num, auth_str)

    # return URL for zip of all output files of canonical instances
    #
    def get_output_files_url(self, batch_id):
        auth_str = hashlib.md5(bytes(self.authenticator+str(batch_id), 'utf-8')).hexdigest()
        return self.url+"get_output.php?cmd=batch_files&batch_id=%d&auth_str=%s"%(batch_id, auth_str)

######### implementation follows ##############

    # send an HTTP post request.
    # the response is XML; return it as a Python dictionary
    #
    def do_http_post(self, req, handler='submit_rpc_handler.php'):
        url = self.url + handler
        params = urllib.parse.urlencode({'request': req})
        params = bytes(params, 'utf-8')
        if self.rpc_timeout>0:
            f = urllib.request.urlopen(url, params, self.rpc_timeout)
        else:
            f = urllib.request.urlopen(url, params)
        reply = f.read()
        root = ElementTree.XML(reply)
        return etree_to_dict(root)

    def create_batch_xml(self, app_name, batch_name, expire_time):
        return ('<create_batch>\n'
            '<authenticator>%s</authenticator>\n'
            '<app_name>%s</app_name>\n'
            '<batch_name>%s</batch_name>\n'
            '<expire_time>%d</expire_time>\n'
            '</create_batch>\n'
        ) %(self.authenticator, app_name, batch_name, expire_time)

    def batch_desc_xml(self, b, op):
        xml = ('<%s>\n'
            '<authenticator>%s</authenticator>\n'
            '<batch>\n'
            '<app_name>%s</app_name>\n'
        ) %(op, self.authenticator, b.app_name)

        if b.batch_id is not None:
            xml += '<batch_id>%s</batch_id>\n'%(b.batch_id)
        elif b.batch_name is not None:
            xml += '<batch_name>%s</batch_name>\n'%(b.batch_name)

        if b.app_version_num is not None:
            xml += '<app_version_num>%d</app_version_num>\n'%(b.app_version_num)

        if b.allocation_priority:
            xml += '<allocation_priority/>\n'
        if b.priority is not None:
            xml += '<priority>%d</priority>\n'%(b.priority)
        for job in b.jobs:
            xml += self.job_desc_xml(job)
        xml += '</batch>\n</%s>\n' %(op)
        return xml

    def job_desc_xml(self, j):
        xml = '<job>\n'
        if j.name is not None:
            xml += '<name>%s</name>\n'%j.name
        if j.rsc_fpops_est is not None:
            xml += '<rsc_fpops_est>%f</rsc_fpops_est>\n'%j.rsc_fpops_est
        if j.command_line is not None:
            xml += '<command_line>%s</command_line>\n'%j.command_line
        if j.input_template is not None:
            xml += '%s\n'%j.input_template
        if j.output_template is not None:
            xml += '%s\n'%j.output_template
        if j.priority is not None:
            xml += '<priority>%d</priority>\n'%(j.priority)
        for file in j.files:
            xml += self.file_desc_xml(file)
        xml += '</job>\n'
        return xml

    def file_desc_xml(self, f):
        xml = '<input_file>\n'
        if f.mode == 'remote':
            xml += '<mode>remote</mode>\n'
            xml += ('<url>%s</url>\n'
                '<nbytes>%f</nbytes>\n'
                '<md5>%s</md5>\n'
            ) %(f.url, f.nbytes, f.md5)
        else:
            xml += '<mode>%s</mode>\n'%(f.mode)
            xml += '<source>%s</source>\n' %(f.source)
        xml += '</input_file>\n'
        return xml

    def batch_op_xml(self, batch_id, op):
        return ('<%s>\n'
            '<authenticator>%s</authenticator>\n'
            '<batch_id>%s</batch_id>\n'
            '</%s>\n'
        ) %(op, self.authenticator, batch_id, op)

    def query_batch_xml(self, batch_id, get_cpu_time, get_job_details):
        return ('<query_batch>\n'
            '<authenticator>%s</authenticator>\n'
            '<batch_id>%s</batch_id>\n'
            '<get_cpu_time>%d</get_cpu_time>\n'
            '<get_job_details>%d</get_job_details>\n'
            '</query_batch>\n'
        ) %(self.authenticator, batch_id,
            1 if get_cpu_time else 0, 1 if get_job_details else 0
        )

    def query_batches_xml(self, get_cpu_time):
        return ('<query_batches>\n'
            '<authenticator>%s</authenticator>\n'
            '<get_cpu_time>%d</get_cpu_time>\n'
            '</query_batches>\n'
        ) %(self.authenticator, 1 if get_cpu_time else 0)

    def query_completed_job_xml(self, job_name):
        return ('<query_completed_job>\n'
        '<authenticator>%s</authenticator>\n'
        '<job_name>%s</job_name>\n'
        '</query_completed_job>\n'
        ) %(self.authenticator, job_name)

    def abort_jobs_xml(self, job_names):
        xml = ('<abort_jobs>\n'
            '<authenticator>%s</authenticator>\n'
            ) %(self.authenticator)
        for job in job_names:
            xml += '<job_name>%s</job_name>\n'%(job)
        xml += '</abort_jobs>\n'
        return xml

    def query_files_xml(self, phys_names, batch_id, delete_time):
        xml = ('<query_files>\n'
            '<authenticator>%s</authenticator>\n'
            '<batch_id>%d</batch_id>\n'
            '<delete_time>%d</delete_time>\n') %(self.authenticator, batch_id, delete_time)
        for name in phys_names:
            xml += '<phys_name>%s</phys_name>\n' %(name)
        xml += '</query_files>\n'
        return xml

    def upload_files_xml(self, phys_names, batch_id, delete_time):
        xml = ('<upload_files>\n'
            '<authenticator>%s</authenticator>\n'
            '<batch_id>%d</batch_id>\n'
            '<delete_time>%d</delete_time>\n') %(self.authenticator, batch_id, delete_time)
        for name in phys_names:
            xml += '<phys_name>%s</phys_name>\n' %(name)
        xml += '</upload_files>\n'
        return xml

# convert ElementTree to a python data structure
#
def etree_to_dict(xml):
    result = {}
    for child in xml:
        if len(child) == 0:
            # this means the element contains data, not other elements
            if child.tag in result:
                if not isinstance(result[child.tag], list):
                    result[child.tag] = [result[child.tag]]
                result[child.tag].append(child.text)
            else:
                result[child.tag] = child.text
        else:
            if child.tag in result:
                if not isinstance(result[child.tag], list):
                    result[child.tag] = [result[child.tag]]
                result[child.tag].append(etree_to_dict(child))
            else:
                result[child.tag] = etree_to_dict(child)
    return result
