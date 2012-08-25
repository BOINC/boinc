## $Id$

'''
Defines database backend library and database table and object relationships.

Example usage:

import database, db_mid

# get platform with id 7; will raise exception if no such platform.
p7 = database.Platforms[7]

# get platforms with friendly name "commodore 64"
p_c64 = database.Platforms.find(user_friendly_name="commodore 64")

# delete results of workunit with name "dead.wu", and email their users:
wu_dead = database.Workunits.find(name="dead.wu")[0]
results_dead = database.Results.find(wu=wu_dead)
for result in results_dead:
    print "Removing from db:", result
    os.system("echo oeps | mail %s" % result.host.user.email_addr)
    result.remove()

# multiply the total_credit of each user by 17:
for user in database.Users.find():
    user.total_credit *= 17
    user.commit()

'''

import configxml
from util import *
from db_base import *

ID = '$Id$'

class Platform(DatabaseObject):
    _table = DatabaseTable(
        table = 'platform',
        columns = [ 'create_time',
                    'name',
                    'user_friendly_name',
                    'deprecated' ])

class App(DatabaseObject):
    _table = DatabaseTable(
        table = 'app',
        columns = [ 'create_time',
                    'name',
                    'min_version',
                    'deprecated',
                    'user_friendly_name',
                    'homogeneous_redundancy',
                    'weight',
                    'beta',
                    'target_nresults',
                    'min_avg_pfc',
                    'host_scale_check',
                    'homogeneous_app_version',
                    'non_cpu_intensive'
                    ])

class AppVersion(DatabaseObject):
    _table = DatabaseTable(
        table = 'app_version',
        columns = [ 'create_time',
                    'appid',
                    'version_num',
                    'platformid',
                    'xml_doc',
                    'min_core_version',
                    'max_core_version',
                    'deprecated',
                    'plan_class',
                    'pfc_n',
                    'pfc_avg',
                    'pfc_scale',
                    'expavg_credit',
                    'expavg_time'
                    ])

class User(DatabaseObject):
    _table = DatabaseTable(
        table = 'user',
        columns = [ 'create_time',
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
                    'posts',
                    'seti_id',
                    'seti_nresults',
                    'seti_last_result_time',
                    'seti_total_cpu',
                    'signature',
                    'has_profile',
                    'cross_project_id',
                    'passwd_hash',
                    'email_validated',
                    'donated'
                    ])

class Team(DatabaseObject):
    _table = DatabaseTable(
        table = 'team',
        columns = [ 'create_time',
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
                    'expavg_credit',
                    'expavg_time',
                    'seti_id',
                    'ping_user',
                    'ping_time'
                    ])

class Host(DatabaseObject):
    _table = DatabaseTable(
        table = 'host',
        columns = [ 'create_time',
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
                    'projects',
                    'nresults_today',
                    'avg_turnaround',
                    'host_cpid',
                    'external_ip_addr',
                    'max_results_day'
                    ])

class Workunit(DatabaseObject):
    _table = DatabaseTable(
        table = 'workunit',
        columns = [ 'create_time',
                    'appid',
                    'name',
                    'xml_doc',
                    'batch',
                    'rsc_fpops_est',
                    'rsc_fpops_bound',
                    'rsc_memory_bound',
                    'rsc_disk_bound',
                    'need_validate',
                    'canonical_resultid',
                    'canonical_credit',
                    'transition_time',
                    'delay_bound',
                    'error_mask',
                    'file_delete_state',
                    'assimilate_state',
                    'hr_class',
                    'opaque',
                    'min_quorum',
                    'target_nresults',
                    'max_error_results',
                    'max_total_results',
                    'max_success_results',
                    'result_template_file',
                    'priority',
                    'mod_time'
                    ])

class Result(DatabaseObject):
    _table = DatabaseTable(
        table = 'result',
        columns = [ 'create_time',
                    'workunitid',
                    'server_state',
                    'outcome',
                    'client_state',
                    'hostid',
                    'userid',
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
                    'client_version_num',
                    'appid',
                    'teamid',
                    'priority',
                    'mod_time'
                    ])


def connect(config = None, nodb = False):
    """Connect if not already connected, using config values."""
    if get_dbconnection():
        return 0
    config = config or configxml.default_config().config
    if nodb:
        db = ''
    else:
        db = config.db_name
    
    host=config.__dict__.get('db_host','')
    do_connect(db=db,
               host=host,
               user=config.__dict__.get('db_user',''),
               passwd=config.__dict__.get('db_passwd', ''))
    return 1

def _execute_sql_script(cursor, filename):
    for query in open(filename).read().split(';'):
        query = query.strip()
        if not query: continue
        cursor.execute(query)

def create_database(srcdir, config = None, drop_first = False):
    ''' creates a new database. '''
    import boinc_path_config
    config = config or configxml.default_config().config
    connect(config, nodb=True)
    cursor = get_dbconnection().cursor()
    if drop_first:
        cursor.execute("drop database if exists %s"%config.db_name)
    cursor.execute("create database %s"%config.db_name)
    cursor.execute("use %s"%config.db_name)
    for file in ['schema.sql', 'constraints.sql']:
        _execute_sql_script(cursor, os.path.join(srcdir, 'db', file))
    cursor.close()

# alias
connect_default_config = connect

database_classes_ = [ Platform,
                      App,
                      AppVersion,
                      User,
                      Team,
                      Host,
                      Workunit,
                      Result ]

Platforms    = Platform._table
Apps         = App._table
AppVersions  = AppVersion._table
Users        = User._table
Teams        = Team._table
Hosts        = Host._table
Workunits    = Workunit._table
Results      = Result._table

init_table_classes(database_classes_,{'canonical_result': Result})
