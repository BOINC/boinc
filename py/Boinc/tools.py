## $Id$

import configxml
try:
    # use new hashlib if available
    from hashlib import md5
except:
    import md5
import os, shutil, binascii, filecmp

def check_immutable(src, dst):
    """If dst exists and is the same as src, return false
       If dst exists and differs from src, throw an exception
       If dst doesn't exist, return true
    """
    if not os.path.exists(dst):
        return True
    if filecmp.cmp(src, dst) == 0:
        raise SystemExit("\nERROR: file "+src+" is different from existing file "+dst+".\nBOINC files are immutable; you must use different names for different files")
    return False

# from http://www.plope.com/software/uuidgen/view
_urandomfd = None
def urandom(n):
    """urandom(n) -> str

    Return a string of n random bytes suitable for cryptographic use.

    """
    global _urandomfd
    if _urandomfd is None:
        try:
            _urandomfd = os.open("/dev/urandom", os.O_RDONLY)
        except:
            _urandomfd = NotImplementedError
    if _urandomfd is NotImplementedError:
        raise NotImplementedError("/dev/urandom (or equivalent) not found")
    bytes = ""
    while len(bytes) < n:
        bytes += os.read(_urandomfd, n - len(bytes))
    return bytes

def make_uuid():
    return binascii.hexlify(urandom(16))

def md5_file(path):
    """
    Return a 16-digit MD5 hex digest of a file's contents
    Read the file in chunks 
    """

    chunk = 8096

    try:
        checksum = md5()
    except NameError:
        checksum = md5.new()

    fp = open(path, 'r')
    while True:
        buffer = fp.read(chunk)
        if not buffer:
            break
        checksum.update(buffer)

    fp.close()

    return checksum

def file_size(path):
    """Return the size of a file"""
    f = open(path)
    f.seek(0,2)
    return f.tell()

def get_kludge_open_name(filename):
    """return part before '=' (or entire filename if none)"""
    assert ('/' not in filename)
    return filename.split('=')[0]

def get_kludge_url_filename(filename):
    """return part after '=' (or entire filename if none)"""
    assert ('/' not in filename)
    if '=' in filename:
        return filename.split('=',1)[1]
    else:
        return filename

def query_sign_executable(executable_path):
    print '''\

SECURITY WARNING:
=================

You have not provided a signature file for %s.

I can generate one now, but this is highly unrecommended.  Generating code
signatures on network-connected computers is a security vulnerability, and
should not be done for publicly-accessable projects.
''' \
        % executable_path

    if not query_noyes('Continue with automatically generating a code signature?'):
        raise SystemExit

def sign_executable(executable_path, quiet=False):
    '''Returns signed text for executable'''
    config = configxml.default_config()
    if not quiet:
        query_sign_executable(executable_path)
        print 'Signing', executable_path
    code_sign_key = os.path.join(config.config.key_dir, 'code_sign_private')

    # sign_executable could be in bin/ or ../tool/, depending on
    # whether this is a test or an upgrade

    sign_executable_path = 'bin/sign_executable'
    if not os.path.exists(sign_executable_path):
        sign_executable_path = '../tools/sign_executable'
        if not os.path.exists(sign_executable_path):
            print os.getcwd()
            raise SystemExit("sign_executable not found! did you `make' it?")
    signature_text = os.popen('%s %s %s'%(sign_executable_path,
        executable_path,code_sign_key)).read()
    if not signature_text:
        raise SystemExit("Couldn't sign executable %s"%executable_path)
    return signature_text

def process_app_file(file, signature_text=None, quiet=False, executable=True):
    '''Handle a new file to be added to the app version.

    0. target filename is url_filename as described in process_app_version
    1. Copy file to download_dir if necessary.
    2. Return <file_info> XML.
        - if signature_text specified, use it
        - if no signature_text specified, generate md5sum.
    '''

    config = configxml.default_config()

    source_dir, source_file_base = os.path.split(file)
    target_file_base = get_kludge_url_filename(source_file_base)
    target_path = os.path.join(config.config.download_dir, target_file_base)
    target_url = os.path.join(config.config.download_url, target_file_base)
    if not quiet:
        print "Copying %s to %s"%(source_file_base, target_path)
    if check_immutable(file, target_path):
        shutil.copy(file, target_path)

    xml = '''<file_info>
    <name>%s</name>
    <url>%s</url>
''' %(target_file_base,target_url)
    if executable:
        xml += '    <executable/>\n'
    if signature_text:
        if signature_text.find('<signatures>') >= 0:
            xml += signature_text
        else:
            xml += '    <file_signature>\n%s    </file_signature>\n'%signature_text
    else:
        xml += '    <md5_cksum>%s</md5_cksum>\n' % md5_file(target_path)

    xml += '    <nbytes>%f</nbytes>\n</file_info>\n' % file_size(target_path)
    return xml

def process_app_version(
    app, version_num, exec_files,
    non_exec_files=[],
    signature_files={},
    file_ref_infos={},
    api_version='',
    extra_xml='',
    quiet=False
    ):
    """Return xml for application version

    app             is an instance of database.App

    version_num     is an integer such as 102 for version 1.02

    exec_files      is a list of full-path executables.
                    exec_file[0] (the first one) is the <main_program/>

    non_exec_files  is a list of full-path non-executables.

    signature_files is a dictionary of exec_file -> signature file mappings.
                    process_app_version() will generate a new signature for
                    any files that don't have one given already.

                    NOTE: using the feature of generating signature files on
                    the same machine (requiring having the private key stored
                    on this machine) is a SECURITY RISK (since this machine
                    probably has network visibility)!

    file_ref_infos  is a dictionary mapping exec_file -> extra XML strings to
                    include in <file_info>, e.g. '<copy_file/>'

    exec_files[1:] and non_exec_files should be named like
    'open_name=url_filename'.
    (url_filename is the basename of file as copied to download/)
    If there is no '=', then the entire filename is used as
    both the open_name and the filename.

    """
    assert(exec_files)
    xml_doc = ''
    for exec_file in exec_files:
        signature_file = signature_files.get(exec_file)
        if signature_file:
            if not quiet: print 'Using signature file', signature_file
            signature_text = open(signature_file).read()
        else:
            signature_text = sign_executable(exec_file, quiet=quiet)
        xml_doc += process_app_file(exec_file, signature_text, quiet=quiet)

    for non_exec_file in non_exec_files:
        signature_file = signature_files.get(non_exec_file)
        if signature_file:
            if not quiet: print 'Using signature file', signature_file
            signature_text = open(signature_file).read()
        else:
            signature_text = sign_executable(non_exec_file, quiet=quiet)
        xml_doc += process_app_file(
            non_exec_file, signature_text=signature_text,
            executable=False, quiet=quiet)

    xml_doc += ('<app_version>\n'+
                     '    <app_name>%s</app_name>\n'+
                     '    <version_num>%d</version_num>\n') %(
        app.name, version_num)

    if (len(api_version)):
        xml_doc += '    <api_version>'+api_version+'</api_version>\n'
    if (len(extra_xml)):
        xml_doc += extra_xml

    first = True
    for exec_file in exec_files + non_exec_files:
        file_base = os.path.basename(exec_file)
        open_name = get_kludge_open_name(file_base)
        url_filename = get_kludge_url_filename(file_base)
        xml_doc += '    <file_ref>\n'
        xml_doc += '       <file_name>%s</file_name>\n' % url_filename
        if first:
            xml_doc += '       <main_program/>\n'
        else:
            xml_doc += '       <open_name>%s</open_name>\n' % open_name
        extra = file_ref_infos.get(exec_file)
        if extra:
            if not extra.endswith('\n'):
                extra += '\n'
            xml_doc += extra
        xml_doc += '    </file_ref>\n'
        first = False

    xml_doc += '</app_version>\n'
    return xml_doc

def query_yesno(str):
    '''Query user; default Yes'''
    print str, "[Y/n] ",
    return not raw_input().strip().lower().startswith('n')

def query_noyes(str):
    '''Query user; default No'''
    print str, "[y/N] ",
    return raw_input().strip().lower().startswith('y')

def get_output_file_path(filename):
    """ Return the filename's path in the upload directory
        Use this if you're developing a validator/assimilator in Python
    """
    config = configxml.default_config()
    fanout = long(config.config.uldl_dir_fanout)
    s = md5.new(filename).hexdigest()[1:8]
    x = long(s, 16)
    return "%s/%x/%s" % (config.config.upload_dir, x % fanout, filename) 
