#!/usr/bin/env python

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
#      -exec_dir b
#      [ -exec_files file1 file2 ... ]
#      [ -signed_exec_files file1 sign1 file2 sign2 ... ]
#      create DB record
#      copy exec to data directory
# add user -email_addr x -name y -authenticator a
#      [ -global_prefs_file y ]

# int version, retval, nexec_files;
# double nbytes;
# bool signed_exec_files;
# char buf[256], md5_cksum[64];
# char *db_name=0, *db_passwd=0, *app_name=0, *platform_name=0;
# char *project_short_name=0, *project_long_name=0;
# char* user_friendly_name=0;
# char* exec_dir=0, *exec_files[10], *signature_files[10];
# char *email_addr=0, *user_name=0, *authenticator=0;
# char *global_prefs_file=0, *download_dir, *download_url;
# char* code_sign_keyfile=0;
# char *message=0, *message_priority=0;

import sys, getopt
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
    [ database.App,        'name', 'min_version', CREATE_TIME],
    [ XAppVersion, 'app', 'platform', 'version_num',
      'signed_exec_file', 'exec_file', 'signature_file'

      ],
    [ database.Platform,   'name', 'user_friendly_name', CREATE_TIME ],
    [ database.User,       'name', 'email_addr', 'authenticator',
      ['?country','United States'], ['?postal_code','94703'],
      '?global_prefs', '?global_prefs_file'
      CREATE_TIME ],
    [ database.Workunit,   'zzzz' ],
    ]

def translate_arg(arg, value,args_dict):
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

    if arg == 'exec_file' or arg == 'signed_exec_file' or arg == 'signature_file':
        args_dict[arg].append(value)
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

class XAppVersion(database.AppVersion):
    def __init__(**kwargs):
        n_signed_file = 0
        signed_exec_file = kwargs['signed_exec_files']
        exec_file = kwargs['exec_files']
        del kwargs['signed_exec_files']
        del kwargs['exec_files']
        for file in signed_exec_files:
            signature_text = signature_files[n_signed_file].read()
            n_signed_file += 1
            kwargs['xml_doc'] = process_executable_file(file, signature_text)
            apply(database.AppVersion.__init__,[],kwargs)
        for file in exec_files:
            signature_text = sign_executable(file)
            kwargs['xml_doc'] = process_executable_file(file, signature_text)
            apply(database.AppVersion.__init__,[],kwargs)

        self.xml_doc += '''<app_version>
    <app_name>%s</app_name>
    <version_num>%d</version_num>''' %(self.name, self.version_num)

        first = True
        for file in exec_files+signed_exec_files:
            if first:
                m = '       <main_program/>\n'
            self.xml_doc += '''
    <file_ref>
         <file_name>%s</filename>
%s    </file_ref>'''%(os.path.basename(file), m)
            first = False
#     for (i=0; i<nexec_files; i++) {
#         sprintf(longbuf,
#             "    <file_ref>\n"
#             "        <file_name>%s</file_name>\n"
#             "%s"
#             "    </file_ref>\n",
#             exec_files[i],
#             i?"":"        <main_program/>\n"
#         );
#         strcat(app_version.xml_doc, longbuf);
#     }
#     strcat(app_version.xml_doc, "</app_version>\n");


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
        (arg,value) = translate_arg(arg,value,args_dict)
        if not arg: continue
        args_dict[arg] = value
    for arg in object.args:
        if not arg in args_dict:
            help_object(object, 'required argument --%s not given'%arg)

    print '### adding %s %s'%(object.name, args_dict)
    object = apply(object.DatabaseObject, [], args_dict)
    object.commit()
    print "Done"

def code_sign_file(executable_path):
    '''Returns signed text for executable'''
    return os.popen('code_sign_file '+executable_path).read()

# void add_app_version() {
#     char path[256];
#     char longbuf[MAX_BLOB_SIZE];
#     int i;

#     strcpy(app_version.xml_doc, "");

#     // copy executables to download directory and sign them
#     //
#     for (i=0; i<nexec_files; i++) {
#         if (signed_exec_files) {
#             read_filename(signature_files[i], signature_text);
#         } else {
#             sprintf(path, "%s/%s", exec_dir, exec_files[i]);
#             sign_executable(path, signature_text);
#         }
#         retval = process_executable_file(
#             exec_files[i], signature_text, app_version.xml_doc
#         );
#         if (retval) {
#             fprintf(stderr, "process_executable_file(): %d\n", retval);
#             exit(1);
#         }
#     }
#     for (i=0; i<nexec_files; i++) {
#         sprintf(longbuf,
#             "    <file_ref>\n"
#             "        <file_name>%s</file_name>\n"
#             "%s"
#             "    </file_ref>\n",
#             exec_files[i],
#             i?"":"        <main_program/>\n"
#         );
#         strcat(app_version.xml_doc, longbuf);
#     }
#     strcat(app_version.xml_doc, "</app_version>\n");

#     app_version.create_time = time(0);
#     retval = app_version.insert();
#     if (retval) {
#         boinc_db_print_error("app_version.insert()");
#         return;
#     }
# }


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


# // copy executable file to the download dir, generate XML
# //
# static int process_executable_file(
#     char* filename, char* signature_text, char* xml_doc
# ) {
#     char longbuf[MAX_BLOB_SIZE];
#     char path[256];

#     sprintf(path, "%s/%s", exec_dir, filename);
#     sprintf(
#         buf,
#         "cp %s %s/%s",
#         path, download_dir, filename
#     );
#     retval = system(buf);
#     if (retval) {
#         printf("failed: %s\n", buf);
#         return retval;
#     }

#     retval = md5_file(path, md5_cksum, nbytes);
#     if (retval) return retval;

#     // generate the XML doc directly.
#     // TODO: use a template, as in create_work (??)
#     //
#     sprintf(longbuf,
#         "<file_info>\n"
#         "    <name>%s</name>\n"
#         "    <url>%s/%s</url>\n"
#         "    <executable/>\n",
#         filename,
#         download_url, filename
#     );
#     strcat(xml_doc, longbuf);
#     if (signature_text) {
#         sprintf(longbuf,
#             "    <file_signature>\n%s"
#             "    </file_signature>\n",
#             signature_text
#         );
#     } else {
#         sprintf(longbuf,
#             "    <md5_cksum>%s</md5_cksum>\n",
#             md5_cksum
#         );
#     }
#     strcat(xml_doc, longbuf);
#     sprintf(longbuf,
#         "    <nbytes>%f</nbytes>\n"
#         "</file_info>\n",
#         nbytes
#     );
#     strcat(xml_doc, longbuf);
#     return 0;
# }

# void add_core_version() {
#     DB_CORE_VERSION core_version;
#     DB_PLATFORM platform;

#     core_version.clear();
#     sprintf(buf, "where name='%s'", platform_name);
#     retval = platform.lookup(buf);
#     if (retval) {
#         fprintf(stderr, "add_core_version(): can't find platform %s\n", platform_name);
#         boinc_db_print_error("platform.lookup()");
#         return;
#     }
#     core_version.platformid = platform.id;
#     core_version.version_num = version;
#     if (message) strcpy(core_version.message, message);
#     if (message_priority) strcpy(core_version.message, message_priority);
#     if (nexec_files != 1) {
#         fprintf(stderr, "add_core_version(): multiple files not allowed\n");
#         return;
#     }
#     strcpy(core_version.xml_doc, "");
#     process_executable_file(exec_files[0], NULL, core_version.xml_doc);
#     core_version.create_time = time(0);
#     retval = core_version.insert();
#     if (retval) {
#         boinc_db_print_error("core_version.insert()");
#     }
# }

# void add_app_version() {
#     char path[256];
#     char longbuf[MAX_BLOB_SIZE];
#     char signature_text[1024];
#     int i;
#     DB_APP app;
#     DB_APP_VERSION app_version;
#     DB_PLATFORM platform;

#     app_version.clear();

#     if (!app_name) {
#         fprintf( stderr, "Application name not specified.\n" );
#         exit(1);
#     }
#     sprintf(buf, "where name='%s'", app_name);
#     retval = app.lookup(buf);
#     if (retval) {
#         fprintf(stderr, "add_app_version(): can't find app %s\n", app_name);
#         boinc_db_print_error("app.lookup()");
#         return;
#     }
#     app_version.appid = app.id;
#     sprintf(buf, "where name='%s'", platform_name);
#     retval = platform.lookup(buf);
#     if (retval) {
#         fprintf(stderr, "add_app_version(): can't find platform %s\n", platform_name);
#         boinc_db_print_error("platform.lookup()");
#         return;
#     }
#     app_version.platformid = platform.id;
#     app_version.version_num = version;

#     strcpy(app_version.xml_doc, "");

#     // copy executables to download directory and sign them
#     //
#     for (i=0; i<nexec_files; i++) {
#         if (signed_exec_files) {
#             read_filename(signature_files[i], signature_text);
#         } else {
#             sprintf(path, "%s/%s", exec_dir, exec_files[i]);
#             sign_executable(path, signature_text);
#         }
#         retval = process_executable_file(
#             exec_files[i], signature_text, app_version.xml_doc
#         );
#         if (retval) {
#             fprintf(stderr, "process_executable_file(): %d\n", retval);
#             exit(1);
#         }
#     }

#     sprintf(longbuf,
#         "<app_version>\n"
#         "    <app_name>%s</app_name>\n"
#         "    <version_num>%d</version_num>\n",
#         app_name,
#         version
#     );
#     strcat(app_version.xml_doc, longbuf);
#     for (i=0; i<nexec_files; i++) {
#         sprintf(longbuf,
#             "    <file_ref>\n"
#             "        <file_name>%s</file_name>\n"
#             "%s"
#             "    </file_ref>\n",
#             exec_files[i],
#             i?"":"        <main_program/>\n"
#         );
#         strcat(app_version.xml_doc, longbuf);
#     }
#     strcat(app_version.xml_doc, "</app_version>\n");

#     app_version.create_time = time(0);
#     retval = app_version.insert();
#     if (retval) {
#         boinc_db_print_error("app_version.insert()");
#         return;
#     }
# }
