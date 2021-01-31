#!/usr/bin/env python

# add_util.py - used in make_project

from __future__ import print_function
from Boinc import database, tools
import time, pprint
import MySQLdb

CREATE_TIME = ['?create_time', int(time.time())]
TRANSITION_TIME = ['?transition_time', int(time.time())]

class XAppVersion(database.AppVersion):
    def __init__(self,**kwargs):
        kwargs['xml_doc'] = tools.process_app_version(
            app = kwargs['app'],
            version_num = int(kwargs['version_num']),
            exec_files = kwargs['exec_files'],
            signature_files = kwargs.setdefault('signature_files',{}))
        del kwargs['signature_files']
        del kwargs['exec_files']
        del kwargs['exec_file']
        database.AppVersion.__init__(self,**kwargs)

# format: [ database.Object, args, ...]
#   arg format:
#       'arg'
#       '?arg'    optional
#       [ 'arg', default_value ]

# NOTE TO KARL: If cross_project_id is not supplied, the default value
# should be a random 32 character script obtained by doing the md5sum
# hash of (say) 512 bytes from /dev/urandom or similar.

list_objects_to_add = [
    [ database.Platform,   'name', 'user_friendly_name', CREATE_TIME ],
    [ database.App,        'name', 'user_friendly_name', ['?min_version',0], CREATE_TIME],
    [ XAppVersion, 'app', 'platform', 'version_num', 'exec_file', '?signature_file',
      CREATE_TIME ],
    [ database.User,       'name', 'email_addr', 'authenticator',
      ['?country','United States'], ['?postal_code','0'], ['?cross_project_id', '0'],
      '?global_prefs', '?global_prefs_file',
      CREATE_TIME ],
    # [ database.Workunit,   'zzzz' ],
    ]

class AddObject:
    pass

add_objects = {}
for o in list_objects_to_add:
    add_object = AddObject()
    add_object.DatabaseObject = o[0]
    add_object.name = add_object.DatabaseObject._table.table
    add_object.args = []
    add_object.optional_args = []
    add_object.default_values = {}
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
            add_object.optional_args.append(arg)
        else:
            add_object.args.append(arg)
        if default_value:
            add_object.default_values[arg] = default_value
    add_objects[add_object.name] = add_object

most_recent_exec_file = None

def translate_arg(object, arg, value, args_dict):
    '''Translate various user argument values, for adding given ``object``.
    Modifies ``args_dict``.'''

    database_table = None
    try:
        database_table = database.__dict__[arg.capitalize()]._table
    except:
        pass
    if database_table:
        args_dict[arg] = translate_database_arg(database_table, arg, value)
        return

    if arg == 'global_prefs_file':
        args_dict['global_prefs'] = open(value).read()
        return

    if object.DatabaseObject == XAppVersion:
        # 'add app_version' accepts multiple '-exec_file's with
        # '-signature_file' applying to the most recent exec_file
        if arg == 'exec_file':
            global most_recent_exec_file
            most_recent_exec_file = value
            args_dict.setdefault('exec_files',[]).append(value)
            # since 'exec_file' (without 's') is required, set it to None so
            # that argument checker knows we got one; we'll delete it later.
            args_dict[arg] = None
            return
        if arg == 'signature_file':
            args_dict.setdefault('signature_files',{})[most_recent_exec_file] = value
            return

    if arg == 'cross_project_id':
        if not value:
            value = tools.make_uuid()

    args_dict[arg] = value

def translate_database_arg(database_table, arg, value):
    '''Look up an object ``value`` either as a database ID or string.
    This allows us to accept e.g. either --app Astropulse or --app 1'''
    try:
        id = int(value)
        results = database_table.find(id=id)
        if not results:
            raise Exception("")
    except:
        results = database_table.find(name=value)
    if len(results) == 0:
        raise SystemExit('No %s "%s" found' %(arg,value))
    if len(results) > 1:
        print('Too many %ss match "%s": '%(arg,value), sys.stderr)
        for result in results:
            print ('   '+result.name, sys.stderr)
        raise SystemExit
    return results[0]

class AddObjectException(Exception): pass

def check_required_arguments(add_object, args_dict):
    for arg in add_object.args:
        if not arg in args_dict:
            raise AddObjectException('required value for %s not given'%arg)

def translate_args_dict(add_object, untranslated_args_dict):
    args_dict = add_object.default_values.copy()
    for arg,value in untranslated_args_dict.items():
        translate_arg(add_object,arg,value,args_dict)
    return args_dict

def exception_is_duplicate_entry(exception):
    '''Checks a MySQLdb.IntegrityError for whether the error is Duplicate
    Entry.  Kludgy.'''
    return (isinstance(exception, MySQLdb.IntegrityError) and
            str(exception).find('Duplicate entry')!=-1)

def do_add_object(add_object, untranslated_args_dict, skip_old=False):
    '''Input ```args_dict``` must have been translated already.'''
    args_dict = translate_args_dict(add_object, untranslated_args_dict)
    check_required_arguments(add_object, args_dict)
    dbobject = add_object.DatabaseObject(**args_dict)
    print("Processing"+dbobject+"...")
    # print "Commiting", dbobject, "with args:"
    # pprint.pprint(dbobject.__dict__)
    try:
        dbobject.commit()
    except MySQLdb.MySQLError as e:
        if skip_old and exception_is_duplicate_entry(e):
            print("  Skipped existing"+dbobject)
            return
        else:
            raise SystemExit('Error committing %s: %s' %(dbobject, e))

    # delete object and re-select it from database to allow user to check
    # consistency
    id = dbobject.id
    del dbobject
    dbobject = add_object.DatabaseObject._table[id]
    print("  Committed"+dbobject+"; values:")
    pprint.pprint(dbobject.__dict__)
