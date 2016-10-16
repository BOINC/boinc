# Python bindings of remote job submission and file management APIs

import urllib
import copy
import xml.etree.ElementTree as ET

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
        xml += '</job>\n';
        return xml

# represents a batch description for submit() or estimate()
#
class BATCH_DESC:
    def __init__(self):
        return

    # convert to XML
    #
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
    r = ET.fromstring(reply)
    return r[0]

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
    return project_url+"/get_output.php?cmd=result_file&result_name=%s&file_num=%s&auth_str=%s"%(name, file_num, auth_str);

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

def query_files(req):
    return do_http_post(req_xml, project_url)

def upload_files(req):
    return do_http_post(req_xml, project_url)

def test_estimate():
    file = FILE_DESC()
    file.mode = 'remote'
    file.url = 'http://isaac.ssl.berkeley.edu/validate_logic.txt'
    file.md5 = "eec5a142cea5202c9ab2e4575a8aaaa7"
    file.nbytes = 4250;

    job = JOB_DESC()
    job.files = [file]

    batch = BATCH_DESC()
    batch.project = 'http://isaac.ssl.berkeley.edu/test/'
    batch.authenticator = "157f96a018b0b2f2b466e2ce3c7f54db"
    batch.app_name = "uppercase"
    batch.batch_name = "blah"
    batch.jobs = []

    for i in range(3):
        job.rsc_fpops_est = i*1e9
        job.command_line = '-i %s' %(i)
        batch.jobs.append(copy.copy(job))

    #print batch.to_xml("submit")
    r = estimate_batch(batch)
    print ET.tostring(r)

def test_query_batches():
    req = REQUEST()
    req.project = 'http://isaac.ssl.berkeley.edu/test/'
    req.authenticator = "157f96a018b0b2f2b466e2ce3c7f54db"
    req.get_cpu_time = True
    r = query_batches(req)
    print ET.tostring(r)

def test_query_batch():
    req = REQUEST()
    req.project = 'http://isaac.ssl.berkeley.edu/test/'
    req.authenticator = "157f96a018b0b2f2b466e2ce3c7f54db"
    req.batch_id = 101
    req.get_cpu_time = True
    r = query_batch(req)
    print ET.tostring(r)

test_query_batch()
