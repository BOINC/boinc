## $Id$

## this module is from quarl's HKN CourseSurvey database.py

import MySQLdb, MySQLdb.cursors
import sys, os
from util import *

ID = '$Id$'

boincdb = None

class DatabaseInconsistency(Exception):
    def __init__(self, descript=None, search_table=None, search_kwargs=None):
        self.descript = descript
        self.search_table = search_table
        self.search_kwargs = search_kwargs
        self.search_tree = []
    def __str__(self):
        return ("""** DATABASE INCONSISTENCY **
   %s
   search_table = %s
   search_kwargs = %s
   search_tree = [
%s
   ] """ %(
            self.descript,
            self.search_table,
            self.search_kwargs,
            '\n'.join(
            map(lambda o:"          %s#%s %s"%(o._table.table,o.__dict__.get('id'),o), self.search_tree))
            ))

class Debug:
    def __init__(self):
        self.html = False
    def printline(self,s):
        if self.html:
            print "<!-- ## %s -->"%s
        else:
            print >>sys.stderr, "##", s

debug = Debug()
debug.mysql = not not os.environ.get('BOINC_DEBUG_DB')

def _commit_object(tablename, paramdict, id=None):
    """Takes a tablename, boincdb object, a parameter dict, and an
        optional id.  Puts together the appropriate SQL command to commit
        the object to the database.  Executes it.  Returns the object's
        id."""
    assert(boincdb)
    cursor = boincdb.cursor()
    equalcommands = []
    for key in paramdict.keys():
        value = paramdict[key]
        if value == None:
            # minicommand = "%s = NULL"%key
            minicommand = ''
            continue
        # elif isinstance(value, str):
        #     minicommand = "%s='%s'" % \
        #         (key, boincdb.escape_string(value))
        # elif isinstance(value, int):
        #     minicommand = "%s=%d" % (key, value)
        # elif isinstance(value, long):
        #     minicommand = "%s=%d" % (key, value)
        # elif isinstance(value, float):
        #     minicommand = "%s=%f" % (key, value)
        # else:
        #     raise Exception('Internal error: unknown type for key %s (value %s type %s)'%(key,value,type(value)))
        else:
            minicommand = "%s='%s'"%(key,boincdb.escape_string(str(value)))
        equalcommands.append(minicommand)
    if id == None:
        command = 'INSERT INTO %s SET %s' % \
            (tablename, ', '.join(equalcommands))
        #print command, '--', paramdict
        if debug.mysql:
            debug.printline("query: "+command)
        cursor.execute(command)
        id = cursor.insert_id()       #porters note: works w/MySQLdb only
    else:
        command =  "UPDATE %s SET %s WHERE id=%d" % \
            (tablename, ', '.join(equalcommands), id)
        if debug.mysql:
            debug.printline("query: "+command)
        cursor.execute(command)
    cursor.close()
    boincdb.commit()
    return id
def _remove_object(command, id=None):
    """Takes a command string, boincdb object, and optional id.  If an
        id is given, it assembles the SQL command and deletes the object
        from the database.  Does nothing if no id is given."""
    assert(boincdb)
    if id == None:
        pass
    else:
        cursor = boincdb.cursor()
        command =  'DELETE FROM ' + command + \
            ' WHERE id=%d' % id
        if debug.mysql:
            debug.printline("query: "+command)
        cursor.execute(command)
        cursor.close()
        boincdb.commit()
def _select_object(table, searchdict, extra_args="", extra_params=[]):
    assert(boincdb)
    parameters = extra_params[:]
    join = None
    if '_join' in searchdict:
        join = searchdict['_join']
        del searchdict['_join']
    if '_extra_params' in searchdict:
        parameters += searchdict['_extra_params']
        del searchdict['_extra_params']
    command = 'SELECT %s.* from %s'%(table,table)
    if join:
        command += "," + join
    for (key,value) in searchdict.items():
        # note: if value == 0, we want to look for it.
        if value != None and value != '':
            # ## need to use isinstance here because Department is derived from str but not str
            # if isinstance(value, str) and key != 'coursenumber':
            #     parameters.append('INSTR(' + key + ', ' + \
            #     "'" + \
            #     boincdb.escape_string(str(value)) \
            #     + "')" )
            # else:
            #     parameters.append("%s='%s'"%(key,value))
            escaped_value = boincdb.escape_string(str(value))
            if key == 'text':
                parameters.append("instr(%s,'%s')"%(key,escaped_value))
            else:
                parameters.append("%s='%s'"%(key,escaped_value))
    if parameters:
        command += ' WHERE ' + ' AND '.join(parameters)
    if extra_args:
        command += ' ' + extra_args.strip()
    cursor = boincdb.cursor()
    if debug.mysql:
        debug.printline("query: "+command)
    cursor.execute(command)
    return cursor

def _select_object_fetchall(*args, **kwargs):
    cursor = apply(_select_object, args, kwargs)
    results = cursor.fetchall()
    cursor.close()
    # if debug.mysql:
    #     print >>sys.stderr, "## results:", results
    return results

# TODO: use iterators/generators for find()
class DatabaseTable:
    def __init__(self, table, columns, extra_columns=[],
                 select_args = None, sort_results = False):
        self.table        = table
        self.lcolumns     = columns
        self.columns      = list2dict(columns)
        self.extra_columns = list2dict(extra_columns)
        # self.object_class = object_class
        self.select_args  = select_args
        self.sort_results = sort_results
        self.objects = {} # mapping from id->object
        # TODO: use a weakref.WeakValueDictionary for self.objects?
        self.defdict = {}
        for key in self.lcolumns:
            if key == 'id':
                self.defdict[key] = None
            elif key.endswith('id'):
                self.defdict[key[:-2]] = None
            elif key.endswith('ids'):
                self.defdict[key[:-3]] = None
            else:
                self.defdict[key] = None
    def find(self, **kwargs):
        """Return a list of database objects matching keywords.

        Allowed keywords are specified by self.columns.

        Objects are cached by ID so repeated lookups are quick.
        """
        if kwargs.keys() == ['id']:
            # looking up by ID only, look in cache first:
            id = kwargs['id']
            if not id:
                return [None]
            try:
                return [self.objects[id]]
            except KeyError:
                pass
            limbo_object = self.object_class(id=None) # prevent possible id recursion
            limbo_object.in_limbo = 1
            self.objects[id] = limbo_object
        kwargs = self.dict2database_fields(kwargs)
        results = _select_object_fetchall(self.table, kwargs,
                                          extra_args=self.select_args)
        objects = self._do_find(kwargs, results)
        if self.sort_results:
            objects.sort()
        return objects
    def _do_find(self, kwargs, results):
        objects = []
        for result in results:
            id = result['id']
            try:
                # object already exists in cache?
                object = self.objects[id]
                if 'in_limbo' in object.__dict__:
                    # we set the object cache so that we don't recurse; update
                    # it with real values and delete the 'in limbo' flag
                    del object.__dict__['in_limbo']
                    object.do_init(result)
            except KeyError:
                # create the object - looking up instructors, etc
                object = apply(self.object_class, [], result)
                if object.id:
                    self.objects[object.id] = object
            objects.append(object)
        return objects
    def find1(self, **kwargs):
        '''Return a single result.  Raises a DatabaseInconsistency if not
        exactly 1 result returned.'''
        objects = apply(self.find, [], kwargs)
        if len(objects) != 1:
            raise DatabaseInconsistency(
                descript="find1: expected 1 result but found %d"%len(objects),
                search_table = self.table,
                search_kwargs = kwargs)
        return objects[0]
    def __getitem__(self, id):
        '''Lookup (possibly cached) object by id.  Returns None if id==None.'''
        return id and self.find1(id=id)
    def objdict2database_fields(self, indict):
        dict = {}
        for key in self.columns:
            if key.endswith('id'):
                obj = indict[key[:-2]]
                dict[key] = obj and obj.id or 0
            else:
                dict[key] = indict[key]
        return dict
    def _valid_query_keys(self):
        return self.columns.keys()+self.extra_columns.keys()+['_join','_extra_params']
    def dict2database_fields(self, indict):
        indict = indict.copy()
        dict = {}
        if 'id' in indict:
            dict['id'] = indict['id']
            del indict['id']
        for key in self._valid_query_keys():
            if key.endswith('id'):
                xkey = key[:-2]
                if xkey in indict:
                    obj = indict[xkey]
                    dict[key] = obj and obj.id
                    del indict[xkey]
            else:
                if key in indict:
                    dict[key] = indict[key]
                    del indict[key]
        if len(indict):
            raise ValueError('Invalid key(s): %s'%indict)
        return dict

class DatabaseObject:
    id_lookups = {} # set near end of file
    def database_fields_to_self(self, dict):
        columns = self._table.columns
        self.__dict__.update(self._table.defdict)  # set defaults to None
        # set this first so that if we get a DatabaseInconsistency we can see
        # the id
        self.id = dict['id']
        for (key, value) in dict.items():
            if key == 'id':
                # self.id = value
                continue
            if not key in columns:
                raise ValueError("database '%s' object doesn't take argument '%s'"%(
                    self._table.table, key))
            if key == 'instructorids':  # KLUDGE
                self.instructors = InstructorList(map(Instructor._table.__getitem__, value))
            elif key == 'instructors': # KLUDGE
                self.instructors = InstructorList(value)
            elif key == 'frequencies': # KLUDGE
                self.frequencies = str_to_dict(value)
            elif key == 'department': # KLUDGE
                self.department = Department(value)
            elif key.endswith('id'):
                xkey = key[:-2]
                self.__dict__[xkey] = self.id_lookups[xkey]._table[value]
            else:
                self.__dict__[key] = value

    def do_init(self, kwargs):
        try:
            self.database_fields_to_self(kwargs)
        except DatabaseInconsistency, e:
            e.search_tree.append(self)
            raise
        # if no id then object starts dirty
        self._set_dirty(not self.id)

    def __init__(self, **kwargs):
        self.do_init(kwargs)

    def __eq__(self, other):
        return other!=None and self.id == other.id
    def __ne__(self, other):
        return not (self == other)
    def __hash__(self):
        return self.id or 0

    def _commit_params(self, paramdict):
        """Commits the object to the boincdb database."""
        self.id = _commit_object(self._table.table, paramdict, self.id)

    def commit(self, force=False):
        if force or self._dirty:
            self._commit_params(self._table.objdict2database_fields(self.__dict__))
            self._set_dirty(False)

    def remove(self):
        """Removes the object from the boincdb database."""
        _remove_object(self._table.table, self.id)
        self.id = None

    def dset(self, key, value):
        if self.__dict__[key] != value:
            self.__dict__[key] = value
            self._set_dirty()
    def __setattr__(self, key, value):
        if key in self._table.columns or key+'id' in self._table.columns:
            self.dset(key, value)
        else:
            self.__dict__[key] = value
    def _set_dirty(self, value=True):
        self.__dict__['_dirty'] = value

        # TODO: move this to db_med
    def URL(self):
        """Relative form of absURL()"""
        return make_url_relative(self.absURL())

class Project(DatabaseObject):
    _table = DatabaseTable(
        table = 'project',
        columns = [ 'id', 'short_name', 'long_name' ] )

class Platform(DatabaseObject):
    _table = DatabaseTable(
        table = 'platform',
        columns = [ 'id',
                    'create_time',
                    'name',
                    'user_friendly_name' ])

class Platform(DatabaseObject):
    _table = DatabaseTable(
        table = 'platform',
        columns = [ 'id',
                    'create_time',
                    'name',
                    'user_friendly_name' ])

class CoreVersion(DatabaseObject):
    _table = DatabaseTable(
        table = 'core_version',
        columns = [ 'id',
                    'create_time',
                    'version_num',
                    'platformid',
                    'xml_doc',
                    'message',
                    'deprecated' ])

class App(DatabaseObject):
    _table = DatabaseTable(
        table = 'app',
        columns = [ 'id',
                    'create_time',
                    'name',
                    'min_version' ])

class AppVersion(DatabaseObject):
    _table = DatabaseTable(
        table = 'app_version',
        columns = [ 'id',
                    'create_time',
                    'appid',
                    'version_num',
                    'platformid',
                    'xml_doc',
                    'min_core_version',
                    'max_core_version' ])

class User(DatabaseObject):
    _table = DatabaseTable(
        table = 'user',
        columns = [ 'id',
                    'create_time',
                    'email_addr',
                    'name',
                    'authenticator',
                    'country',
                    'postal_code',
                    'total_credit',
                    'expavg_credit',
                    'expavg_time',
                    'global_prefs',
                    'project_prefs',
                    'teamid',
                    'venue',
                    'url',
                    'send_email',
                    'show_hosts',
                    'posts' ])

class Team(DatabaseObject):
    _table = DatabaseTable(
        table = 'team',
        columns = [ 'id',
                    'create_time',
                    'userid',
                    'name',
                    'name_lc',
                    'url',
                    'type',
                    'name_html',
                    'description',
                    'nusers',
                    'country',
                    'total_credit',
                    'expavg_credit' ])


class Host(DatabaseObject):
    _table = DatabaseTable(
        table = 'host',
        columns = [ 'id',
                    'create_time',
                    'userid',
                    'rpc_seqno',
                    'rpc_time',
                    'total_credit',
                    'expavg_credit',
                    'expavg_time',
                    'timezone',
                    'domain_name',
                    'serialnum',
                    'last_ip_addr',
                    'nsame_ip_addr',
                    'on_frac',
                    'connected_frac',
                    'active_frac',
                    'p_ncpus',
                    'p_vendor',
                    'p_model',
                    'p_fpops',
                    'p_iops',
                    'p_membw',
                    'os_name',
                    'os_version',
                    'm_nbytes',
                    'm_cache',
                    'm_swap',
                    'd_total',
                    'd_free',
                    'd_boinc_used_total',
                    'd_boinc_used_project',
                    'd_boinc_max',
                    'n_bwup',
                    'n_bwdown',
                    'credit_per_cpu_sec',
                    'venue',
                    'projects' ])

class Workunit(DatabaseObject):
    _table = DatabaseTable(
        table = 'workunit',
        columns = [ 'id',
                    'create_time',
                    'appid',
                    'name',
                    'xml_doc',
                    'batch',
                    'rsc_fpops',
                    'rsc_iops',
                    'rsc_memory',
                    'rsc_disk',
                    'need_validate',
                    'canonical_resultid',
                    'canonical_credit',
                    'transition_time',
                    'delay_bound',
                    'error_mask',
                    'file_delete_state',
                    'assimilate_state',
                    'workseq_next',
                    'opaque',
                    'min_quorum',
                    'target_nresults',
                    'max_error_results',
                    'max_total_results',
                    'max_success_results',
                    'result_template' ])

class Result(DatabaseObject):
    _table = DatabaseTable(
        table = 'result',
        columns = [ 'id',
                    'create_time',
                    'workunitid',
                    'server_state',
                    'outcome',
                    'client_state',
                    'hostid',
                    'report_deadline',
                    'sent_time',
                    'received_time',
                    'name',
                    'cpu_time',
                    'xml_doc_in',
                    'xml_doc_out',
                    'stderr_out',
                    'batch',
                    'file_delete_state',
                    'validate_state',
                    'claimed_credit',
                    'granted_credit',
                    'opaque',
                    'random',
                    'client_version_num' ])

class Workseq(DatabaseObject):
    _table = DatabaseTable(
        table = 'workseq',
        columns = [ 'id',
                    'create_time',
                    'state',
                    'hostid',
                    'wuid_last_done',
                    'wuid_last_sent',
                    'workseqid_master' ])

def _connectp(dbname, user, passwd, host='localhost'):
    """Takes a database name, a username, and password.  Connects to
    SQL server and makes a new Boincdb."""
    global boincdb
    if boincdb:
        raise 'Already connected'
    boincdb = MySQLdb.connect(db=dbname,host=host,user=user,passwd=passwd,
                               cursorclass=MySQLdb.cursors.DictCursor)

def _connectm(module):
    _connectp(module.database, module.username, module.password)

def connect(readonly = False):
    """Connect if not already connected or if we're adding write permissions"""
    global boincdb
    if boincdb:
        if not readonly and boincdb.readonly:
            # re-open with write access
            boincdb.close()
            boincdb = None
        else:
            return 0
    if readonly:
        import password_settings_r
        _connectm(password_settings_r)
    else:
        import password_settings
        _connectm(password_settings)
    boincdb.readonly = readonly
    return 1

def close():
    """Closes the connection to the sql boinc and deletes the Boincdb object."""
    boincdb.close()

database_classes = [ Project,
                     Platform,
                     CoreVersion,
                     App,
                     AppVersion,
                     User,
                     Team,
                     Host,
                     Workunit,
                     Result,
                     Workseq ]

for Class in database_classes:
    # these couldn't be defined earlier because the classes weren't defined yet.
    Class._table.object_class = Class
    DatabaseObject.id_lookups[Class._table.table] = Class

DatabaseObject.id_lookups['canonical_result'] = Result

Projects = Project._table
Platforms = Platform._table
CoreVersions = CoreVersion._table
Apps = App._table
AppVersions = AppVersion._table
Users = User._table
Teams = Team._table
Hosts = Host._table
Workunits = Workunit._table
Results = Result._table
Workseqs = Workseq._table

database_tables = map(lambda c: c._table, database_classes)

# def check_database_consistency():
#     '''Raises DatabaseInconsistency on error.

#     Loads the entire database into memory so will take a while.
#     '''
#     for table in database_tables:
#         print 'Checking', table.table
#         print '   checked', len(table.find())
