## $Id$

import boinc_path_config
import configxml
import os, md5, shutil

def md5_file(path):
    """Return a 16-digit MD5 hex digest of a file's contents"""
    return md5.new(open(path).read()).hexdigest()

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

def sign_executable(executable_path, quiet=False):
    '''Returns signed text for executable'''
    config = configxml.default_config()
    if not quiet: print 'Signing', executable_path
    code_sign_key = os.path.join(config.config.key_dir, 'code_sign_private')
    sign_executable_path = os.path.join(boinc_path_config.TOP_BUILD_DIR,
                                        'tools','sign_executable')
    if not os.path.exists(sign_executable_path):
        raise SystemExit("tools/sign_executable not found! did you `make' it?")
    signature_text = os.popen('%s %s %s'%(sign_executable_path,
                                          executable_path,code_sign_key)).read()
    if not signature_text:
        raise SystemExit("Couldn't sign executable %s"%executable_path)
    return signature_text

def process_executable_file(file, signature_text=None, quiet=False, executable=True):
    '''Handle a new executable (or non-executable) file to be added to the
    database.

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
    if file != target_path:
        if not quiet:
            print "Copying %s to %s"%(source_file_base, target_path)
        shutil.copy(file, target_path)

    xml = '''<file_info>
    <name>%s</name>
    <url>%s</url>
''' %(source_file_base,target_url)
    if executable:
        xml += '    <executable/>\n'
    if signature_text:
        xml += '    <file_signature>\n%s    </file_signature>\n'%signature_text
    else:
        xml += '    <md5_cksum>%s</md5_cksum>\n' % md5_file(target_path)

    xml += '    <nbytes>%f</nbytes>\n</file_info>\n' % file_size(target_path)
    return xml

def process_app_version(app, version_num, exec_files, non_exec_files=[], signature_files={}, quiet=False):
    """Return xml for application version

    app             is an instance of database.App

    version_num     is an integer such as 102 for version 1.02

    exec_files      is a list of full-path executables.
                    exec_file[0] (the first one) is the <main_program/>

    non_exec_files  is a list of full-path non-executables.

    signature_files is a dictionary of exec_file -> signature file mappings.
                    process_app_version() will generate a new signature for
                    any exec_files that don't have one given already.

                    NOTE: using the feature of generating signature files on
                    the same machine (requiring having the private key stored
                    on this machine) is a SECURITY RISK (since this machine
                    probably has network visibility)!

    exec_files[1:] and non_exec_files should be named like 'open_name=url_filename'.
                    (url_filename is the basename of file as copied to
                    download/) If there is no '=', then the entire filename is
                    used as both the open_name and the filename.

    """
    assert(exec_files)
    xml_doc = ''
    for exec_file in exec_files:
        signature_file = signature_files.get(exec_file)
        if signature_file:
            signature_text = open(signature_file).read()
        else:
            signature_text = sign_executable(exec_file, quiet=quiet)
        xml_doc += process_executable_file(exec_file, signature_text, quiet=quiet)

    for non_exec_file in non_exec_files:
        # use MD5 sum instead of RSA signature
        xml_doc += process_executable_file(non_exec_file, signature_text=None,
                                           executable=False, quiet=quiet)

    xml_doc += ('<app_version>\n'+
                     '    <app_name>%s</app_name>\n'+
                     '    <version_num>%d</version_num>\n') %(
        app.name, version_num)

    first = True
    for exec_file in exec_files + non_exec_files:
        file_base = os.path.basename(exec_file)
        open_name = get_kludge_open_name(file_base)
        url_filename = get_kludge_url_filename(file_base)
        xml_doc += '    <file_ref>\n'
        xml_doc += '       <file_name>%s</filename>\n' % url_filename
        if first:
            xml_doc += '       <main_program/>\n'
        else:
            xml_doc += '       <open_name>%s</open_name>\n' % open_name
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
