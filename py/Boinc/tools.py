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

def sign_executable(executable_path):
    '''Returns signed text for executable'''
    config = configxml.default_config()
    print 'Signing', executable_path
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

def process_executable_file(file, signature_text=None):
    '''Handle a new executable file to be added to the database.

    1. Copy file to download_dir if necessary.
    2. Return <file_info> XML.
        - if signature_text specified, include it; else generate md5sum.
    '''

    config = configxml.default_config()

    file_dir, file_base = os.path.split(file)
    target_path = os.path.join(config.config.download_dir, file_base)
    if file_dir != config.config.download_dir:
        print "Copying %s to %s"%(file_base, config.config.download_dir)
        shutil.copy(file, target_path)

    xml = '''<file_info>
    <name>%s</name>
    <url>%s</url>
    <executable/>
''' %(file_base,
      os.path.join(config.config.download_url, file_base))

    if signature_text:
        xml += '    <file_signature>\n%s    </file_signature>\n'%signature_text
    else:
        xml += '    <md5_cksum>%s</md5_cksum>\n' % md5_file(target_path)

    xml += '    <nbytes>%f</nbytes>\n</file_info>\n' % file_size(target_path)
    return xml

def process_app_version(app, version_num, exec_files, signature_files={}):
    """Return xml for application version

    app             is an instance of database.App

    version_num     is an integer such as 102 for version 1.02

    exec_files      is a list of full-path executables

    signature_files is a dictionary of exec_file -> signature file mappings.
                    process_app_version() will generate a new signature for
                    any exec_files that don't have one given already.
    """
    assert(exec_files)
    xml_doc = ''
    for exec_file in exec_files:
        signature_file = signature_files.get(exec_file)
        if signature_file:
            signature_text = open(signature_file).read()
        else:
            signature_text = sign_executable(exec_file)
        xml_doc += process_executable_file(exec_file, signature_text)

    xml_doc += ('<app_version>\n'+
                     '    <app_name>%s</app_name>\n'+
                     '    <version_num>%d</version_num>\n') %(
        app.name, version_num)

    first = True
    for exec_file in exec_files:
        xml_doc += ('    <file_ref>\n'+
                         '       <file_name>%s</filename>\n') %(
            os.path.basename(exec_file))
        if first:
            xml_doc += '       <main_program/>\n'

        xml_doc += '    </file_ref>\n'
        first = False

    xml_doc += '</app_version>\n'
    return xml_doc
