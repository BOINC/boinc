#!/usr/bin/env python

# $Id$

# The contents of this file are subject to the BOINC Public License
# Version 1.0 (the "License"); you may not use this file except in
# compliance with the License. You may obtain a copy of the License at
# http://boinc.berkeley.edu/license_1.0.txt
#
# Software distributed under the License is distributed on an "AS IS"
# basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
# License for the specific language governing rights and limitations
# under the License.
#
# The Original Code is the Berkeley Open Infrastructure for Network Computing.
#
# The Initial Developer of the Original Code is the SETI@home project.
# Portions created by the SETI@home project are Copyright (C) 2002
# University of California at Berkeley. All Rights Reserved.
#
# Contributor(s):
#

# add - add items to the DB
#
# usages:
# add project -project_short_name x -project_long_name x
#      add project
# add app -app_name x
#      add application
#      create DB record
# add platform -platform_name x -user_friendly_name y
#      create DB record
# add app_version
#      -app_name x -platform_name y -version a
#      -download_dir d -download_url e
#      -exec_file file1 [ -signature_file signature1 ]
#      -exec_file file2 [ -signature_file signature2 ]
#   file1 is the main programs.  signature files should be created on another
#   machine but for testing purposes add.py will sign for you (for a real
#   project you should never store the private key on the networked server)
#      create DB record
#      copy exec to data directory
# add user -email_addr x -name y -authenticator a
#      [ -global_prefs_file y ]

import sys, getopt, md5
sys.path.append('../py/')
import database, db_mid
from util import *

CREATE_TIME = ['?create_time', time.time()]

# format: [ database.Object, args, ...]
#   arg format:
#       'arg'
#       '?arg'    optional
#       [ 'arg', default_value ]
list_objects_to_add = [
    [ database.Project,    'name', '?long_name' ],
    [ database.Platform,   'name', 'user_friendly_name', CREATE_TIME ],
    [ XCoreVersion, 'platform', 'version_num', 'exec_file',
      ['?message',''], ['?message_priority',''],
    [ database.App,        'name', 'min_version', CREATE_TIME],
    [ XAppVersion, 'app', 'platform', 'version_num', 'exec_file', '?signature_file'
      ],
      CREATE_TIME ],
    [ database.User,       'name', 'email_addr', 'authenticator',
      ['?country','United States'], ['?postal_code','94703'],
      '?global_prefs', '?global_prefs_file'
      CREATE_TIME ],
    # [ database.Workunit,   'zzzz' ],
    ]

most_recent_exec_file = None

def translate_arg(object, arg, value, args_dict):
    '''Translate various arguments'''
    database_table = None
    try:
        database_table = database.__dict__[arg.capitalize]._table
    except:
        pass
    if database_table:
        return (arg,translate_database_arg(database_table, arg, value))

    if arg == 'global_prefs_file':
        return ('global_prefs', open(value).read())

    if object.DatabaseObject == XAppVersion:
        # 'add app_version' accepts multiple '-exec_file's with
        # '-signature_file' applying to the most recent exec_file
        if arg == 'exec_file':
            args_dict['exec_files'].append(value)
            # since this is required, set it to None so that argument checker
            # knows we got one; we'll delete it later.
            return (arg,None)
        if arg == 'signature_file':
            args_dict['signature_files'][most_recent_exec_file] = value
            return (None,None)

    return (arg,value)


def translate_database_arg(database_table, arg, value):
    '''Accept e.g. either --app Astropulse or --app 1'''
    try:
        id = int(value)
        results = database_table.find(id=id)
        if not results:
            raise Exception("")
    except:
        results = database_table.find(name=value)
    if len(results) == 0:
        raise SystemExit('No %s %s found' %(arg,value))
    if len(results) > 1:
        raise SystemExit('Too many %s match "%s"'%(arg,value))
    return results[0]

class XCoreVersion(database.CoreVersion):
    def __init__(**kwargs):
        exec_file = kwargs['exec_file']
        del kwargs['exec_file']
        kwargs['xml_doc'] = process_executable_file(exec_file)
        apply(database.CoreVersion.__init__,[],kwargs)

class XAppVersion(database.AppVersion):
    def __init__(**kwargs):
        signature_files = kwargs['signature_files']
        exec_files = kwargs['exec_files']
        if not exec_files:
            raise Exception('internal error: no exec_files - should have caught this earlier')
        del kwargs['signature_files']
        del kwargs['exec_files']
        del kwargs['exec_file']
        xml_doc = ''
        for exec_file in exec_files
            signature_file = signature_files.get(exec_file)
            if signature_file:
                signature_text = open(signature_file).read()
            else:
                signature_text = sign_executable(exec_file)
            xml_doc += process_executable_file(exec_file, signature_text)

        xml_doc += ('<app_version>\n'+
                         '    <app_name>%s</app_name>\n'+
                         '    <version_num>%d</version_num>\n') %(
            self.name, self.version_num)

        first = True
        for exec_file in exec_files:
            xml_doc += ('    <file_ref>\n'+
                             '         <file_name>%s</filename>\n') %(
                os.path.basename(exec_file))
            if first:
                xml_doc += '       <main_program/>\n'

            xml_doc += '    </file_ref>\n'
            first = False

        xml_doc += '</app_version>\n'
        kwargs['xml_doc'] = xml_doc
        apply(database.AppVersion.__init__,[],kwargs)


def ambiguous_lookup(string, dict):
    results = []
    string = string.replace('_','')
    for key in dict:
        k = key.replace('_','')
        if k == string:
            return [k]
        if k.startswith(string):
            results.append(dict[key])
    return results

def parse_global_options(args):
    # raise SystemExit('todo')
    pass

def dv(object,arg):
    if arg in object.default_args:
        return '    --%s [%s]' %(arg, object.default_args[arg])
    else:
        return '    --%s' %arg

def help_object(object, msg=None):
    if msg:
        print >>sys.stderr, msg
        print
    print >>sys.stderr, "Syntax: add %s"%object.name
    for arg in object.args:
        print >>sys.stderr, dv(object,arg)
    print >>sys.stderr, " Optional:"
    for arg in object.optional_args:
        print >>sys.stderr, dv(object,arg)
    raise SystemExit

def add_object(object, args):
    try:
        parsed_opts, placement_args = \
                     getopt.getopt(args, '',
                                   map(lambda s: s+'=',
                                       object.args + object.optional_args))
        if placement_args:
            raise 'Unknown args '+' '.join(placement_args)
    except Exception, e:
        help_object(object, e)
    args_dict = {}
    for arg,value in parsed_opts:
        if not arg.startswith('--'):
            raise Exception('internal error: arg should start with "--"')
        arg = arg[2:]
        (arg,value) = translate_arg(object,arg,value,args_dict)
        if not arg: continue
        args_dict[arg] = value
    for arg in object.args:
        if not arg in args_dict:
            help_object(object, 'required argument --%s not given'%arg)

    object = apply(object.DatabaseObject, [], args_dict)
    object.commit()
    print "Done"

def code_sign_file(executable_path):
    '''Returns signed text for executable'''
    print 'Signing', executable_path
    return os.popen('sign_executable %s %s'%(executable_path,config.config.code_sign_key)).read()

class Dict:
    pass

objects_to_add = {}
for o in list_objects_to_add:
    object = Dict()
    object.DatabaseObject = o[0]
    object.name = object.DatabaseObject._table.table
    object.args = []
    object.optional_args = []
    object.default_values = {}
    for arg in o[1:]:
        if isinstance(arg, list):
            default_value = arg[1]
            arg = arg[0]
        else:
            default_value = None
        if arg.startswith('?'):
            optional = True
            arg = arg[1:]
        else:
            optional = False
        if optional:
            objects.optional_args.append(arg)
        else:
            objects.args.append(arg)
        if default_value:
            object.default_values[arg] = default_value
    objects_to_add[object.name] = object

if len(sys.argv) < 2:
    print >>sys.stderr, """Syntax: add <object_to_add> <options...>

Adds an object to the BOINC database.

Objects to add:"""
    for object in sorted_keys(objects_to_add):
        print >>sys.stderr, "    ", object
    print >>sys.stderr, """
Global options:
     --db_name           Database name
     --db_password       Database password [optional]
     --db_user           Database user     [optional]

For command-line help on a particular object, use add <object> without further
arguments.
"""

    raise SystemExit(1)

name_of_object_to_add = sys.argv[1].strip().lower()

possible_objects = ambiguous_lookup(name_of_object_to_add, objects_to_add)
if len(possible_objects) == 0:
    raise SystemExit("No such object '%s' to add"%name_of_object_to_add)
if len(possible_objects) > 1:
    print >>sys.stderr, "Object name '%s' matches multiple objects:"%name_of_object_to_add
    for object in possible_objects:
        print "    ", object.name
    raise SystemExit(1)

args = sys.argv[2:]
parse_global_options(args)
add_object(possible_objects[0], args)


def md5_file(path):
    """Return a 16-digit MD5 hex digest of a file's contents"""
    return md5.new(open(path).read()).hexdigest()

def file_size(path):
    """Return the size of a file"""
    f = open(path)
    f.seek(0,2)
    return f.tell()

def process_executable_file(file, signature_text=None):
    '''Handle a new executable file to be added to the database.

    1. Copy file to download_dir if necessary.
    2. Return <file_info> XML.
        - if signature_text specified, include it; else generate md5sum.
    '''

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
