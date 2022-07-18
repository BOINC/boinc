# quarl 2003-10-16 initial version based on conglomeration of
#                  coursesurvey/database.py and boinc/database.py

# quarl 2003-10-16 implemented lazy lookups

# DB_BASE - an awesome view of an SQL database in Python.  All relational
# objects are lazily cached.  I.e. if table WORKUNIT has field RESULTID, wu =
# database.Workunits.find1(name='Wu1') will look up Wu1; accessing wu.result
# will do a database.Results.find1(id=wu.resultid) the first time

from __future__ import generators
import MySQLdb, MySQLdb.cursors
import sys, os, weakref

dbconnection = None

def list2dict(list):
    dict = {}
    for k in list: dict[k] = None
    return dict

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
            [ "          %s#%s %s"%(o._table.table,o.__dict__.get('id'),o) for o in self.search_tree ]
            )))

class Debug:
    def __init__(self):
        self.html = False
    def printline(self,s):
        if self.html:
            print("<!-- ## %s -->"%s)
        else:
            print("##"+s, sys.stderr)

debug = Debug()
debug.mysql = not not os.environ.get('DEBUG_DB')

def _execute_sql(cursor, command):
    '''Same as ``cursor.execute(command)``, but more verbose on error.'''
    try:
        cursor.execute(command)
    except MySQLdb.MySQLError as e:
        e.args += (command,)
        raise e

def _commit_object(tablename, paramdict, id=None):
    """Takes a tablename, a parameter dict, and an optional id.  Puts together
    the appropriate SQL command to commit the object to the database.
    Executes it.  Returns the object's id."""
    assert(dbconnection)
    cursor = dbconnection.cursor()
    equalcommands = []
    for key in paramdict.keys():
        value = paramdict[key]
        if value is None:
            continue
        elif isinstance(value, int):
            equalcommands.append('%s=%d' %(key,value))
        else:
            equalcommands.append("%s='%s'"%(key,dbconnection.escape_string(str(value))))
    if id is None:
        command = 'INSERT INTO %s SET %s' % \
            (tablename, ', '.join(equalcommands))
        if debug.mysql:
            debug.printline("query: "+command)
        _execute_sql(cursor, command)
        # id = cursor.insert_id()       #porters note: works w/MySQLdb only
        id = cursor.lastrowid
    else:
        command =  "UPDATE %s SET %s WHERE id=%d" % \
            (tablename, ', '.join(equalcommands), id)
        if debug.mysql:
            debug.printline("query: "+command)
        _execute_sql(cursor, command)
    cursor.close()
    dbconnection.commit()
    return id
def _remove_object(command, id=None):
    """Takes a command string, dbconnection object, and optional id.  If an
        id is given, it assembles the SQL command and deletes the object
        from the database.  Does nothing if no id is given."""
    assert(dbconnection)
    if id is None:
        pass
    else:
        cursor = dbconnection.cursor()
        command =  'DELETE FROM ' + command + \
            ' WHERE id=%d' % id
        if debug.mysql:
            debug.printline("query: "+command)
        _execute_sql(cursor, command)
        cursor.close()
        dbconnection.commit()
def _select_object(table, searchdict, extra_args="", extra_params=[], select_what=None):
    assert(dbconnection)
    parameters = extra_params[:]
    join = None
    if '_join' in searchdict:
        join = searchdict['_join']
        del searchdict['_join']
    if '_extra_params' in searchdict:
        parameters += searchdict['_extra_params']
        del searchdict['_extra_params']
    command = 'SELECT %s from %s'%((select_what or "%s.*"%table) ,table)
    if join:
        command += "," + join
    for (key,value) in searchdict.items():
        if value is None:
            value = ''
        escaped_value = dbconnection.escape_string(str(value))
        if key == 'text':
            parameters.append("instr(%s,'%s')"%(key,escaped_value))
        else:
            parameters.append("%s='%s'"%(key,escaped_value))
    if parameters:
        command += ' WHERE ' + ' AND '.join(parameters)
    if extra_args:
        command += ' ' + extra_args.strip()
    cursor = dbconnection.cursor()
    if debug.mysql:
        debug.printline("query: "+command)
    _execute_sql(cursor, command)
    return cursor

def _select_object_fetchall(*args, **kwargs):
    cursor = _select_object(*args, **kwargs)
    results = cursor.fetchall()
    cursor.close()
    return results

def _select_object_iterate(*args, **kwargs):
    cursor = _select_object(*args, **kwargs)
    while True:
        result = cursor.fetchone()
        if not result: return
        yield result

def _select_count_objects(*args, **kwargs):
    kwargs['select_what'] = 'count(*)'
    cursor = _select_object(*args, **kwargs)
    result = cursor.fetchone().values()[0]
    cursor.close()
    return result

class Options:
    pass

options = Options()

# keep up to this many objects in cache.  we use a very bone-headed
# cache-management algorithm: when we reach this many objects, drop the oldest
# half.
options.OBJECT_CACHE_SIZE = 1024

# don't lookup referenced Ids until they are asked for.  I.e., if
# evaluatedclass.instructorteamid=123, then if instructorteam#123 hasn't been
# looked up yet, don't look up evaluatedclass.instructorteam until it is
# referenced.  use DEBUG_DB=1 to check out this niftiness.
options.LAZY_LOOKUPS = True

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
        ## self.objects is a mapping from id->object which weakly references
        ## all current objects from this table.  this guarantees that if a
        ## find() returns a row for which we already created an object in
        ## memory, we return the same one.
        self.objects = weakref.WeakValueDictionary()
        ## self.object_cache is a list of the N most recently retrieved
        ## objects.  its values aren't really used; the list is used to ensure
        ## the strong reference count for self.objects[object] is nonzero to
        ## ensure is lifetime.
        ##
        ## This means if you look up database.Apps[1]: the first lookup does a
        ## MySQL SELECT; afterwards database.Apps[1] is free (only requires a
        ## lookup in database.Apps.objects).  To prevent this object cache
        ## from growing without bound, database.Apps.object is a weak-valued
        ## dictionary.  This means that if no one refers to the object, it is
        ## deleted from the dictionary.  database.Apps.object_cache maintains
        ## a list of the OBJECT_CACHE_SIZE most recent lookups, which forces
        ## their strong reference count.
        self.object_cache = []
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

    def _cache(self, object):
        """Maintain up to OBJECT_CACHE_SIZE objects in the object_cache list.

        The object's existence in this cache ensures its strong reference
        count is nonzero, so that it doesn't get implicitly dropped from
        self.objects."""
        if len(self.object_cache) >= options.OBJECT_CACHE_SIZE:
            self.object_cache = self.object_cache[:-options.OBJECT_CACHE_SIZE/2]
        self.object_cache.append(object)

    def _is_cached(self, id):
        '''Returns True if object is automatically-cached, i.e. in the weak
        reference cache.

        This is not the same as the manual cache invoked by _cache(), i.e. the
        strong reference cache.
        '''
        return id in self.objects

    def _modify_find_args(self, kwargs):
        '''Derived classes can override this function to modify kwargs.

        This is only called for non-trivial find args (if there are arguments
        and not just "id")'''
        pass

    def clear_cache(self):
        """
        Clears the cached objects list
        """
        self.object_cache = []

    def count(self, **kwargs):
        """Return the number of database objects matching keywords.

        Arguments are the same format as find()."""
        if not kwargs:
            # shortcut since this is the most common case
            return _select_count_objects(self.table, {})
        if kwargs.keys() == ['id']:
            # looking up by ID only, look in cache first:
            id = kwargs['id']
            if not id:
                return 0
            if id in self.objects:
                return 1
        self._modify_find_args(kwargs)
        kwargs = self.dict2database_fields(kwargs)
        return _select_count_objects(self.table, kwargs,
                                     extra_args=self.select_args)
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
            self._cache(limbo_object)
        self._modify_find_args(kwargs)
        kwargs = self.dict2database_fields(kwargs)
        results = _select_object_fetchall(self.table, kwargs,
                                          extra_args=self.select_args)
        objects = self._create_objects_from_sql_results(results, kwargs)
        # up to this point objects should be equivalent to list(iterate(...))
        if self.sort_results:
            objects.sort()
        return objects
    def iterate(self, **kwargs):
        """Same as find(), but using generators, and no sorting."""
        if kwargs.keys() == ['id']:
            # looking up by ID only, look in cache first:
            id = kwargs['id']
            if not id:
                return
            try:
                yield self.objects[id]
                return
            except KeyError:
                pass
            limbo_object = self.object_class(id=None) # prevent possible id recursion
            limbo_object.in_limbo = 1
            self.objects[id] = limbo_object
            self._cache(limbo_object)
        self._modify_find_args(kwargs)
        kwargs = self.dict2database_fields(kwargs)
        for result in _select_object_iterate(self.table, kwargs,
                                             extra_args=self.select_args):
            yield self._create_object_from_sql_result(result)
        return

    def _create_objects_from_sql_results(self, results, kwargs):
        return [ self._create_object_from_sql_result(result) for result in results ]

    def _create_object_from_sql_result(self, result):
        id = result['id']
        try:
            # object already exists in cache?
            object = self.objects[id]
            if 'in_limbo' in object.__dict__:
                # earlier we set the object cache so that we don't recurse;
                # update it now with real values and delete the 'in limbo'
                # flag
                del object.__dict__['in_limbo']
                object.do_init(result)
        except KeyError:
            # create the object - looking up instructors, etc
            object = self.object_class(**result)
            if object.id:
                self.objects[object.id] = object
                self._cache(object)
        return object

    def find1(self, **kwargs):
        '''Return a single result.  Raises a DatabaseInconsistency if not
        exactly 1 result returned.'''
        objects = self.find(**kwargs)
        if len(objects) != 1:
            raise DatabaseInconsistency(
                descript="find1: expected 1 result but found %d"%len(objects),
                search_table = self.table,
                search_kwargs = kwargs)
        return objects[0]
    def __getitem__(self, id):
        '''Lookup (possibly cached) object by id.  Returns None if id==None.'''
        return id and self.find1(id=id)
    def objdict2database_fields(self, indict, lazydict):
        dict = {}
        for key in self.columns:
            if key.endswith('id'):
                xkey = key[:-2]
                if xkey in lazydict:
                    # lazydict maps 'name' (without 'id') -> (table,id)
                    dict[key] = lazydict[xkey][1]
                else:
                    obj = indict[xkey]
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
    id_lookups = {} # set by init_table_classes
    def _set_field(self, key, value):
        """Set field KEY to VALUE.  May be overridden by derived class.

        if options.LAZY_LOOKUPS is true, then possibly don't look up a value
        yet.
        """
        if key.endswith('id') and not key.endswith('_id'):
            xkey = key[:-2]
            table = self.id_lookups[xkey]._table
            id = value
            if options.LAZY_LOOKUPS:
                if table._is_cached(id):
                    self.__dict__[xkey] = table.objects[id]
                else:
                    del self.__dict__[xkey]
                    self._lazy_lookups[xkey] = (table, id)
            else:
                # always lookup values
                self.__dict__[xkey] = table[id]
        else:
            self.__dict__[key] = value

    def __getattr__(self, name):
        if options.LAZY_LOOKUPS and name in self._lazy_lookups:
            (table, id) = self._lazy_lookups[name]
            del self._lazy_lookups[name]
            object = table[id]          # this probably invokes MySQL SELECTs
            self.__dict__[name] = object
            return object
        raise AttributeError(name)

    def database_fields_to_self(self, dict):
        columns = self._table.columns
        self.__dict__.update(self._table.defdict)  # set defaults to None
        # set this first so that if we get a DatabaseInconsistency we can see
        # the id
        self.id = dict.get('id')
        for (key, value) in dict.items():
            if key == 'id':
                continue
            if key or key+'id' in columns:
                self._set_field(key, value)
            else:
                raise ValueError("database '%s' object doesn't take argument '%s'"%(
                    self._table.table, key))

    def do_init(self, kwargs):
        try:
            self.database_fields_to_self(kwargs)
        except DatabaseInconsistency as e:
            e.search_tree.append(self)
            raise
        # if no id then object starts dirty
        self._set_dirty(not self.id)

    def __init__(self, **kwargs):
        self._lazy_lookups = {}
        self.do_init(kwargs)

    def __eq__(self, other):
        return other!=None and isinstance(other, DatabaseObject) and self.id == other.id
    def __ne__(self, other):
        return not (self == other)
    def __hash__(self):
        return self.id or 0

    def _commit_params(self, paramdict):
        """Commits the object to the dbconnection database."""
        self.id = _commit_object(self._table.table, paramdict, self.id)

    def commit(self, force=False):
        if force or self._dirty:
            self._commit_params(self._table.objdict2database_fields(self.__dict__, self._lazy_lookups))
            self._set_dirty(False)

    def remove(self):
        """Removes the object from the dbconnection database."""
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

def do_connect(db, user, passwd, port, host='localhost'):
    """Takes a database name, a username, and password.  Connects to
    SQL server and makes a new Dbconnection."""
    global dbconnection
    if dbconnection:
        raise 'Already connected'
    dbconnection = MySQLdb.connect(db=db, host=host, port=port, user=user,
        passwd=passwd, cursorclass=MySQLdb.cursors.DictCursor)

def close():
    """Closes the connection to the sql survey and deletes the Dbconnection object."""
    global dbconnection
    dbconnection.close()
    dbconnection = None

def get_dbconnection():
    return dbconnection
def set_dbconnection(d):
    global dbconnection
    dbconnection = d

def init_table_classes(database_classes_, more_id_lookups = {}):
    """initialize the list of database classes and tables.  To be called from
    database.py.
    """

    global database_classes, database_tables
    database_classes = database_classes_
    for Class in database_classes:
        Class._table.object_class = Class
        DatabaseObject.id_lookups[Class._table.table] = Class

    DatabaseObject.id_lookups.update(more_id_lookups)

    database_tables = [ c._table for c in database_classes ]

def check_database_consistency():
    '''Raises DatabaseInconsistency on error.

    Loads the entire database into memory so will take a while.
    '''
    options.LAZY_LOOKUPS = False
    for table in database_tables:
        print('\rChecking %s: [counting]' %(table.table))
        sys.stdout.flush()
        count = table.count()
        i = 0
        j_limit = int(count / 100) # show progress every 1%
        j = j_limit
        print('\rChecking %s: [iterating]' %(table.table))
        sys.stdout.flush()
        for object in table.iterate():
            # we don't need to do anything here; just iterating through the
            # database will automatically read everything into memory
            i += 1
            if j == j_limit:
                print('\rChecking %s: [%d/%d] %3.f%%' %(table.table, i, count, 100.0*i/count))
                sys.stdout.flush()
                j = 0
            j += 1
        print('\rChecking %s: all %d rows are good' %(table.table, count))
