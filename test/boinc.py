## $Id$

# boinc.py
#
# A set of classes for testing BOINC.  These classes let you create multiple
# projects and multiple hosts (all running on a single machine), add
# applications and work units, run the system, and verify that the results are
# correct.
#
# See doc/test.html for details

# the module MySQLdb can be installed on debian with "apt-get install python2.2-mysqldb"

from version import *
from boinc_db import *
import os, sys, time, shutil, re, atexit, traceback, random
import MySQLdb

errors = 0

def get_env_var(name, default = None):
    try:
        return os.environ.get(name, default)
    except KeyError:
        print "Environment variable %s not defined" % name
        sys.exit(1)

# VERBOSE: 0 = print nothing
#          1 = print some, and overwrite lines (default)
#          2 = print all

VERBOSE = int(get_env_var('TEST_VERBOSE', 1))

def verbose_echo(level, line):
    if level == 0:
        if VERBOSE == 1:
            print
        print line
    elif VERBOSE >= level:
        if VERBOSE >= 2:
            print line
        elif VERBOSE == 1:
            print "\r                                                                               ",
            print "\r", line,
            sys.stdout.flush()

def fatal_error(msg):
    global errors
    errors += 1
    verbose_echo(0, "FATAL ERROR: "+msg)
    sys.exit(1)

def error(msg, fatal=0):
    global errors
    if fatal: fatal_error(msg)
    errors += 1
    verbose_echo(0, "ERROR: "+msg)

def verbose_sleep(msg, wait):
    front = msg + ' [sleep '
    back = ']'
    for i in range(1,wait+1):
        verbose_echo(1, msg + ' [sleep ' + ('.'*i).ljust(wait) + ']')
        time.sleep(1)

def shell_call(cmd, failok=False):
    if os.system(cmd):
        error("Command failed: "+cmd, fatal=(not failok))
        return 1
    return 0

def verbose_shell_call(cmd, failok=False):
    verbose_echo(2, "   "+cmd)
    return shell_call(cmd, failok)

def proxerize(url, t):
    if t:
        r = re.compile('http://[^/]*/')
        return r.sub('http://localhost:080/', url)
    else:
        return url

KEY_DIR      = get_env_var("BOINC_KEY_DIR")
# SHMEM_KEY    = get_env_var("BOINC_SHMEM_KEY")
PROJECTS_DIR = get_env_var("BOINC_PROJECTS_DIR")
CGI_URL      = get_env_var("BOINC_CGI_URL")
HTML_URL     = get_env_var("BOINC_HTML_URL")
USER_NAME    = get_env_var("BOINC_USER_NAME", '') or get_env_var("USER")
CGI_DIR      = get_env_var("BOINC_CGI_DIR")
HTML_DIR     = get_env_var("BOINC_HTML_DIR")
HOSTS_DIR    = get_env_var("BOINC_HOSTS_DIR")

def use_cgi_proxy():
    CGI_URL = proxerize(CGI_URL)
def use_html_proxy():
    HTML_URL = proxerize(HTML_URL)

def check_exists(file):
    if not os.path.isfile(file):
        error("file doesn't exist: " + file)
        return 1
    return 0
def check_deleted(file):
    if os.path.isfile(file):
        error("file wasn't deleted: " + file)
        return 1
    return 0
def check_files_match(file, correct, descr=''):
    if not os.path.isfile(file):
        error("file doesn't exist: %s (needs to match %s)" % (file,correct))
        return 1
    if os.system("diff %s %s" % (file, correct)):
        error("File mismatch%s: %s %s" % (descr, file, correct))
        return 1
    else:
        verbose_echo(2, "Files match%s: %s %s" % (descr, file, correct))
        return 0

def check_program_exists(prog):
    if not os.path.isfile(prog):
        fatal_error("""
Executable not found: %s
Did you `make' yet?
""" % prog)
def check_core_client_executable():
    check_program_exists(os.path.join(SRC_DIR, 'client', CLIENT_BIN_FILENAME))
def check_app_executable(app):
    check_program_exists(os.path.join(SRC_DIR, 'apps', app))

def macro_substitute(macro, replacement, infile, outfile):
    open(outfile, 'w').write(open(infile).read().replace(macro, replacement))
def macro_substitute_inplace(macro, replacement, inoutfile):
    old = inoutfile + '.old'
    os.rename(inoutfile, old)
    macro_substitute(macro, replacement, old, inoutfile)

def make_executable(name):
    os.chmod(name, 755)
def symlink(src, dest):
    if os.path.exists(dest):
        os.unlink(dest)
    os.symlink(src, dest)
def rmtree(dir):
    try:
        shutil.rmtree(dir)
    except OSError:
        pass

def _url_to_filename(url):
    return url.replace('http://','').replace('/','_')
def account_file_name(url):
    return 'account_' + _url_to_filename(url) + '.xml'

def run_tool(cmd):
    verbose_shell_call(os.path.join(SRC_DIR, 'tools', cmd))

def _gen_key_p(private_key, public_key):
    shell_call("%s/crypt_prog -genkey 1024 %s %s" % (
        os.path.join(SRC_DIR, '/lib'),
        os.path.join(KEY_DIR, private_key),
        os.path.join(KEY_DIR, public_key)))
def _gen_key(key):
    _gen_key_p(key+'_private', key+'_public')
def create_keys():
    _gen_key('upload')
    _gen_key('code_sign')

def unique(list):
    d = {}
    for i in list:
        d[i] = 1
    return d.keys()

def map_xml(dic, keys):
    if not isinstance(dic,dict):
        dic = dic.__dict__
    s = ''
    for key in keys:
        s += "<%s>%s</%s>\n" % (key, dic[key], key)
    return s[:-1]

def generate_shmem_key():
    return '0x1111%x' % random.randrange(0,2**16)

class STARTS_WITH(str):
    pass

def dict_match(dic, resultdic):
    '''match values in DIC against RESULTDIC'''
    if not isinstance(dic, dict):
        dic = dic.__dict__
    for key in dic.keys():
        expected = dic[key]
        try:
            found = resultdic[key]
        except KeyError:
            error("Database query result didn't have key '%s'!" % key)
            continue
        if isinstance(expected,STARTS_WITH):
            match = found.startswith(expected)
        else:
            match = found == expected
        if not match:
            id = resultdic.get('id', '?')
            if str(found).count('\n') or str(expected).count('\n'):
                format = """result %s: unexpected %s:

%s

(expected:)

%s"""
            else:
                format = "result %s: unexpected %s '%s' (expected '%s')"
            error( format % (id, key, found, expected))

def _db_query(db, query):
    db.query(query)
    result = db.use_result()
    return result and result.fetch_row(0,1)

class Platform:
    def __init__(self, name, user_friendly_name=None):
        self.name = name
        self.user_friendly_name = user_friendly_name or name

class CoreVersion:
    def __init__(self):
        self.version = 1
        self.platform = Platform(PLATFORM)
        self.exec_dir = os.path.join(SRC_DIR, 'client')
        self.exec_name = CLIENT_BIN_FILENAME

class App:
    def __init__(self, name):
        assert(name)
        self.name = name

class AppVersion:
    def __init__(self, app, version = 1):
        self.exec_names = []
        self.exec_dir = os.path.join(SRC_DIR, 'apps')
        self.exec_names = [app.name]
        self.app = app
        self.version = 1
        self.platform = Platform(PLATFORM)

class Project:
    def __init__(self, works, users=None, hosts=None,
                 short_name=None, long_name=None, core_versions=None,
                 apps=None, app_versions=None, appname=None):
        self.short_name = short_name or 'test_'+appname
        self.long_name = long_name or 'Project ' + self.short_name.replace('_',' ').capitalize()
        self.db_passwd = ''
        self.generate_keys = False
        self.shmem_key = generate_shmem_key()
        self.resource_share = 1
        self.output_level = 3

        self.master_url    = os.path.join(HTML_URL         , self.short_name , '')
        self.download_url  = os.path.join(HTML_URL         , self.short_name , 'download')
        self.cgi_url       = CGI_URL
        self.upload_url    = os.path.join(self.cgi_url     , self.short_name , 'file_upload_handler')
        self.scheduler_url = os.path.join(self.cgi_url     , self.short_name , 'cgi')
        self.project_dir   = os.path.join(PROJECTS_DIR     , self.short_name)
        self.download_dir  = os.path.join(self.project_dir , 'download')
        self.upload_dir    = os.path.join(self.project_dir , 'upload')
        self.key_dir       = os.path.join(self.project_dir , 'keys')
        self.user_name     = USER_NAME
        self.db_name       = self.user_name + '_' + self.short_name
        self.project_php_file = None
        self.project_prefs_php_file = None

        self.core_versions = core_versions or [CoreVersion()]
        self.app_versions = app_versions or [AppVersion(App(appname))]
        self.apps = apps or unique(map(lambda av: av.app, self.app_versions))
        self.platforms = [Platform(PLATFORM)]
        self.works = works
        self.users = users or [User()]
        self.hosts = hosts or [Host()]
        # convenience vars:
        self.app_version = self.app_versions[0]
        self.app = self.apps[0]
        self.host = self.hosts[0]
        self.work = self.works[0]
        self.user = self.users[0]

    def srcdir(self, *dirs):
        return apply(os.path.join,(SRC_DIR,)+dirs)
    def dir(self, *dirs):
        return apply(os.path.join,(self.project_dir,)+dirs)

    def mkdir(self, dir):
        os.mkdir(self.dir(dir))
    def chmod(self, dir):
        os.chmod(self.dir(dir), 0777)
    def copy(self, source, dest):
        shutil.copy(self.srcdir(source), self.dir(dest))
    def copytree(self, source, dest = None, failok=False):
        shutil.copytree(self.srcdir(source),
                        self.dir(dest or os.path.join(source, '')))

    def run_db_script(self, script):
        shell_call('sed -e s/BOINC_DB_NAME/%s/ %s | mysql'
                   % (self.db_name, self.srcdir('db', script)))
    def db_open(self):
        return MySQLdb.connect(db=self.db_name)

    def install_project(self, scheduler_file = None):
        verbose_echo(1, "Deleting previous test runs")
        rmtree(self.dir())

        verbose_echo(1, "Setting up server: creating directories");
        # make the CGI writeable in case scheduler writes req/reply files

        map(self.mkdir,
            [ '', 'cgi', 'upload', 'download', ])#'keys', 'html_ops', 'html_user' ])
        map(self.chmod,
            [ '', 'cgi', 'upload' ])

        if self.generate_keys:
            verbose_echo(1, "Setting up server files: generating keys");
            print "KEY GENERATION NOT IMPLEMENTED YET"
        else:
            verbose_echo(1, "Setting up server files: copying keys");
            self.copytree(KEY_DIR, self.key_dir)

        # copy the user and administrative PHP files to the project dir,
        verbose_echo(1, "Setting up server files: copying html directories")

        self.copytree('html_user')
        self.copytree('html_ops')
        self.copy('tools/country_select', 'html_user')
        if self.project_php_file:
            self.copy(os.path.join('html_user', self.project_php_file),
                      os.path.join('html_user', 'project.inc'))
        if self.project_prefs_php_file:
            self.copy(os.path.join('html_user', self.project_prefs_php_file),
                      os.path.join('html_user', 'project_specific_prefs.inc'))

        symlink(self.download_dir, self.dir('html_user', 'download'))

        # Copy the sched server in the cgi directory with the cgi names given
        # source_dir/html_usr/schedulers.txt
        #

        verbose_echo(1, "Setting up server files: copying cgi programs");
        r = re.compile('<scheduler>([^<]+)</scheduler>', re.IGNORECASE)
        if scheduler_file:
            f = open(self.dir('html_user', scheduler_file))
            for line in f:
                # not sure if this is what the scheduler file is supposed to
                # mean
                match = r.search(line)
                if match:
                    cgi_name = match.group(1)
                    verbose_echo(2, "Setting up server files: copying " + cgi_name);
                    copy('sched/cgi', os.path.join('cgi', cgi_name))
            f.close()
        else:
            scheduler_file = 'schedulers.txt'
            f = open(self.dir('html_user', scheduler_file), 'w')
            print >>f, "<scheduler>" + self.scheduler_url, "</scheduler>"
            f.close()


        # copy all the backend programs to the CGI directory
        map(lambda (s): self.copy(os.path.join('sched', s), 'cgi/'),
            [ 'cgi', 'file_upload_handler', 'make_work',
              'feeder', 'timeout_check', 'validate_test',
              'file_deleter', 'assimilator', 'start_servers', 'kill_server' ])

        verbose_echo(1, "Setting up database")
        map(self.run_db_script, [ 'drop.sql', 'schema.sql', 'constraints.sql' ])

        db = self.db_open()
        db.query("insert into project(short_name, long_name) values('%s', '%s')" %(
            self.short_name, self.long_name));

        verbose_echo(1, "Setting up database: adding %d user(s)" % len(self.users))
        for user in self.users:
            if user.project_prefs:
                pp = "<project_preferences>\n%s\n</project_preferences>\n" % user.project_prefs
            else:
                pp = ''
            if user.global_prefs:
                gp = "<global_preferences>\n%s\n</global_preferences>\n" % user.global_prefs
            else:
                gp = ''

            db.query(("insert into user values (0, %d, '%s', '%s', '%s', " +
                      "'Peru', '12345', 0, 0, 0, '%s', '%s', 0, 'home', '', 0, 1)") % (
                time.time(),
                user.email_addr,
                user.name,
                user.authenticator,
                gp,
                pp))

        verbose_echo(1, "Setting up database: adding %d apps(s)" % len(self.apps))
        for app in self.apps:
            check_app_executable(app.name)
            db.query("insert into app(name, create_time) values ('%s', %d)" %(
                app.name, time.time()))

        self.platforms = unique(map(lambda a: a.platform, self.app_versions))
        verbose_echo(1, "Setting up database: adding %d platform(s)" % len(self.platforms))

        db.close()

        for platform in self.platforms:
            run_tool("add platform -db_name %s -platform_name %s -user_friendly_name '%s'" %(
                self.db_name, platform.name, platform.user_friendly_name))

        verbose_echo(1, "Setting up database: adding %d core version(s)" % len(self.core_versions))
        for core_version in self.core_versions:
            run_tool(("add core_version -db_name %s -platform_name %s" +
                      " -version %s -download_dir %s -download_url %s -exec_dir %s" +
                      " -exec_files %s") %
                     (self.db_name, core_version.platform.name,
                      core_version.version,
                      self.download_dir,
                      self.download_url,
                      core_version.exec_dir,
                      core_version.exec_name))

        verbose_echo(1, "Setting up database: adding %d app version(s)" % len(self.app_versions))
        for app_version in self.app_versions:
            app = app_version.app
            cmd = ("add app_version -db_name %s -app_name '%s'" +
                   " -platform_name %s -version %s -download_dir %s -download_url %s" +
                   " -code_sign_keyfile %s -exec_dir %s -exec_files") % (
                self.db_name, app.name, app_version.platform.name,
                app_version.version,
                self.download_dir,
                self.download_url,
                os.path.join(self.key_dir, 'code_sign_private'),
                app_version.exec_dir)
            for exec_name in app_version.exec_names:
                cmd += ' ' + exec_name
            run_tool(cmd)

        verbose_echo(1, "Setting up server files: writing config files");

        config = map_xml(self,
                         [ 'db_name', 'db_passwd', 'shmem_key',
                           'key_dir', 'download_url', 'download_dir',
                           'upload_url', 'upload_dir', 'project_dir', 'user_name',
                           'output_level' ])
        self.append_config(config)

        # put a file with the database name and other info in each HTML
        # directory

        htconfig = self.dir('html_user', '.htconfig.xml')
        htconfig2 = self.dir('html_ops', '.htconfig.xml')
        f = open(htconfig, 'w')
        print >>f, map_xml(self,
                           [ 'db_name', 'db_passwd', 'download_url', 'cgi_url'] )
        f.close()
        shutil.copy(htconfig, htconfig2)

        # edit "index.php" in the user HTML directory to have the right file
        # as the source for scheduler_urls; default is schedulers.txt

        macro_substitute_inplace('FILE_NAME', scheduler_file,
                                 self.dir('html_user', 'index.php'))

        # create symbolic links to the CGI and HTML directories
        verbose_echo(1, "Setting up server files: linking cgi programs");
        symlink(self.dir('cgi'), os.path.join(CGI_DIR, self.short_name))
        symlink(self.dir('html_user'), os.path.join(HTML_DIR, self.short_name))
        symlink(self.dir('html_ops'), os.path.join(HTML_DIR, self.short_name+'_admin'))

        # show the URLs for user and admin sites
        admin_url = os.path.join("html_user", self.short_name+'_admin/')

        verbose_echo(2, "Master URL: " + self.master_url)
        verbose_echo(2, "Admin URL:  " + admin_url)

    def install(self):
        self.install_project()
        for work in self.works:
            work.install(self)
        for host in self.hosts:
            for user in self.users:
                host.add_user(user, self)
            host.install()

    def http_password(self, user, password):
        'Adds http password protection to the html_ops directory'
        passwd_file = self.dir('html_ops', '.htpassword')
        f = open(self.dir('html_ops', '.htaccess'), 'w')
        print >>f, "AuthName '%s Administration'" % self.long_name
        print >>f, "AuthType Basic"
        print >>f, "AuthUserFile %s" % passwd_file
        print >>f, "require valid-user"
        f.close()
        shell_call("htpassword -bc %s %s %s" % (passwd_file, user, password))

    def _disable(self, *path):
        '''Temporarily disable a file to test exponential backoff'''
        path = apply(self.dir, path)
        os.rename(path, path+'.disabled')
    def _reenable(self, *path):
        path = apply(self.dir, path)
        os.rename(path+'.disabled', path)

    def disable_masterindex(self):
        self._disable('html_user/index.php')
    def reenable_masterindex(self):
        self._reenable('html_user/index.php')
    def disable_scheduler(self, num = ''):
        self._disable('cgi/cgi'+str(num))
    def reenable_scheduler(self, num = ''):
        self._reenable('cgi/cgi'+str(num))
    def disable_downloaddir(self, num = ''):
        self._disable('download'+str(num))
    def reenable_downloaddir(self, num = ''):
        self._reenable('download'+str(num))
    def disable_file_upload_handler(self, num = ''):
        self._disable('cgi/file_upload_handler'+str(num))
    def reenable_file_upload_handler(self, num = ''):
        self._reenable('cgi/file_upload_handler'+str(num))


    def _run_cgi_prog(self, prog, args='', logfile=None):
        verbose_shell_call("cd %s && ./%s %s >> %s.out 2>&1" %
                           (self.dir('cgi'), prog, args, (logfile or prog)))
    def _run_cgi_onepass(self, prog, args=''):
        self._run_cgi_prog(prog, '-d 3 -one_pass '+args)
    def start_servers(self):
        self.restart()
        self._run_cgi_prog('start_servers')
        verbose_sleep("Starting servers for project '%s'" % self.short_name, 1)

    def _install_prog(self, prog, args=''):
        self.append_config('<start>./%s -d 3 -asynch %s >>%s.out 2>&1</start>' % (
            prog, args, prog))
    def install_feeder(self):
        self._install_prog('feeder')
    def install_timeout_check(self, app, nerror=5, ndet=5, nredundancy=5):
        self._install_prog('timeout_check', '-app %s -nerror %d -ndet %d -nredundancy %d' %(
            app, nerror, ndet, nredundancy))
    def install_make_work(self, work, cushion, redundancy):
        self._install_prog('make_work', '-cushion %d -redundancy %d -result_template %s -wu_name %s' %(
            cushion, redundancy,
            os.path.realpath(work.result_template),
            work.wu_template))
    def uninstall_make_work(self):
        self.remove_config('make_work')
    def install_validate(self, app, quorum):
        for app_version in self.app_versions:
            self._install_prog('validate_test', '-app %s -quorum %d' %(
                app_version.app.name, quorum))
    def validate(self, quorum):
        for app_version in self.app_versions:
            self._run_cgi_onepass('validate_test', '-app %s -quorum %d' %(
                app_version.app.name, quorum))
    def install_file_delete(self):
        self._install_prog('file_deleter')
    def file_delete(self):
        self._run_cgi_onepass('file_deleter')
    def install_assimilator(self):
        for app_version in self.app_versions:
            self._install_prog('assimilator', '-app %s' % app_version.app.name)
    def assimilate(self):
        for app_version in self.app_versions:
            self._run_cgi_onepass('assimilator', '-app %s' % app_version.app.name)

    def start_stripcharts(self):
        map(lambda l: self.copy(os.path.join('stripchart', l), 'cgi/'),
            [ 'stripchart.cgi', 'stripchart', 'stripchart.cnf',
              'looper', 'db_looper', 'datafiles', 'get_load', 'dir_size' ])
        macro_substitute('BOINC_DB_NAME', self.db_name, self.srcdir('stripchart/samples/db_count'),
                         self.dir('cgi/db_count'))
        make_executable(self.dir('cgi/db_count'))

        self._run_cgi_prog('looper'    , 'get_load 1'                            , 'get_load')
        self._run_cgi_prog('db_looper' , '"result" 1'                            , 'count_results')
        self._run_cgi_prog('db_looper' , '"workunit where assimilate_state=2" 1' , 'assimilated_wus')
        self._run_cgi_prog('looper'    , '"dir_size ../download" 1'              , 'download_size')
        self._run_cgi_prog('looper'    , '"dir_size ../upload" 1'                , 'upload_size')

    def stop(self, daemons=['ALL']):
        '''Stop running scheduler daemons.'''
        ## the following shouldn't be necessary anymore:
        f = open(self.dir('cgi', 'stop_server'), 'w')
        print >>f, "<quit/>"
        f.close()

        daemons = ' '.join(daemons)
        verbose_echo(1,"Stopping server(s) for project '%s': "%self.short_name)
        shell_call("cd %s ; ./kill_server -v %s" % (self.dir('cgi'), daemons))

    def restart(self):
        '''remove the stop_server trigger'''
        try:
            os.unlink(self.dir('cgi', 'stop_server'))
        except OSError:
            pass

    configlines = []
    def append_config(self, line):
        self.configlines += line.split('\n')
        self.write_config()

    def write_config(self):
        f = open(self.dir('cgi/.htconfig.xml'), 'w')
        print >>f, '<config>'
        for line in self.configlines:
            print >>f, "   ", line
        print >>f, '</config>'

    def remove_config(self, pattern):
        self.configlines = filter(lambda l: l.find(pattern)==-1, self.configlines)
        self.write_config()

    def check_results(self, ntarget, matchresult):
        '''MATCHRESULT should be a dictionary of columns to check, such as:

        server_state
        stderr_out
        exit_status
        '''
        db = self.db_open()
        rows = _db_query(db,"select * from result")
        for row in rows:
            dict_match(matchresult, row)
        db.close()
        if len(rows) != ntarget:
            error("expected %d results, but found %d" % (ntarget, len(rows)))

    def num_wus_left(self, db):
        return _db_query(db, "select count(*) from result where server_state=%d"%RESULT_SERVER_STATE_UNSENT)[0]['count(*)']
    def num_results_done(self, db):
        return _db_query(db, "select count(*) from result where server_state=%d"%RESULT_SERVER_STATE_OVER)[0]['count(*)']

    def check_files_match(self, result, correct, count=None):
        '''if COUNT is specified then [0,COUNT) is mapped onto the %d in RESULT'''
        if count != None:
            errs = 0
            for i in range(count):
                errs += self.check_files_match(result%i, correct)
            return errs
        return check_files_match(self.dir(result),
                                 correct, " for project '%s'"%self.short_name)
    def check_deleted(self, file, count=None):
        if count != None:
            errs = 0
            for i in range(count):
                errs += self.check_deleted(file%i)
            return errs
        return check_deleted(self.dir(file))
    def check_exists(self, file, count=None):
        if count != None:
            errs = 0
            for i in range(count):
                errs += self.check_exists(file%i)
            return errs
        return check_exists(self.dir(file))

class User:
    '''represents an account on a particular project'''
    def __init__(self):
        self.name = 'John'
        self.email_addr = 'john@boinc.org'
        self.authenticator = "3f7b90793a0175ad0bda68684e8bd136"
        self.project_prefs = None
        self.global_prefs = None

class Host:
    def __init__(self):
        self.name = 'Commodore64'
        self.users = []
        self.projects = []
        self.global_prefs = None
        self.log_flags = 'log_flags.xml'
        self.host_dir = os.path.join(HOSTS_DIR, self.name)
        self.defargs = "-exit_when_idle -skip_cpu_benchmarks"

    def add_user(self, user, project):
        self.users.append(user)
        self.projects.append(project)

    def dir(self, *dirs):
        return apply(os.path.join,(self.host_dir,)+dirs)

    def install(self):
        rmtree(self.dir())
        os.mkdir(self.dir())

        verbose_echo(1, "Setting up host '%s': creating account files" % self.name);
        for (user,project) in map(None,self.users,self.projects):
            filename = self.dir(account_file_name(project.master_url))
            verbose_echo(2, "Setting up host '%s': writing %s" % (self.name, filename))

            f = open(filename, "w")
            print >>f, "<account>"
            print >>f, map_xml(project, ['master_url'])
            print >>f, map_xml(user, ['authenticator'])
            if user.project_prefs:
                print >>f, user.project_prefs
            print >>f, "</account>"
            f.close()

        # copy log flags and global prefs, if any
        if self.log_flags:
            shutil.copy(self.log_flags, self.dir('log_flags.xml'))
        if self.global_prefs:
            shell_call("cp %s %s" % (self.global_prefs, self.dir('global_prefs.xml')))
            # shutil.copy(self.global_prefs, self.dir('global_prefs.xml'))

    def run(self, args='', asynch=False):
        if asynch:
            verbose_echo(1, "Running core client asynchronously")
            pid = os.fork()
            if pid: return pid
        else:
            verbose_echo(1, "Running core client")
        verbose_shell_call("cd %s && %s %s %s > client.out" % (
            self.dir(), os.path.join(SRC_DIR, 'client', CLIENT_BIN_FILENAME),
            self.defargs, args))
        if asynch: os._exit(0)

    def read_cpu_time_file(filename):
        try:
            return float(open(self.dir(filename)).readline())
        except:
            return 0

    def check_file_present(self, project, filename):
        check_exists(self.dir('projects',
                              _url_to_filename(project.master_url),
                              filename))

class Work:
    def __init__(self):
        self.input_files = []
        self.rsc_iops = 1.8e12
        self.rsc_fpops = 1e13
        self.rsc_memory = 1e7
        self.rsc_disk = 1e7
        self.delay_bound = 1000
        self.redundancy = 1
        self.app = None

    def install(self, project):
        verbose_echo(1, "Installing work <%s> in project '%s'" %(
            self.wu_template, project.short_name))
        if not self.app:
            self.app = project.app_versions[0].app
        for input_file in self.input_files:
            shutil.copy(input_file, project.download_dir)

        # simulate multiple data servers by making symbolic links to the
        # download directory
        r = re.compile('<download_url/>([^<]+)<', re.IGNORECASE)
        for line in open(self.wu_template):
            match = r.search(line)
            if match:
                newdir = project.download_dir+match.group(1)
                verbose_echo(2, "Linking "+newdir)
                os.symlink(project.download_dir, newdir)

        # simulate multiple data servers by making copies of the file upload
        # handler
        r = re.compile('<upload_url/>([^<]+)<', re.IGNORECASE)
        for line in open(self.wu_template):
            match = r.search(line)
            if match:
                handler = project.srcdir('sched', 'file_upload_handler')
                newhandler = handler + match.group(1)
                verbose_echo(2, "Linking "+newhandler)
                os.symlink(handler, newhandler)

        cmd = "create_work -db_name %s -download_dir %s -upload_url %s -download_url %s -keyfile %s -appname %s -rsc_iops %.0f -rsc_fpops %.0f -rsc_disk %.0f -wu_template %s -result_template %s -redundancy %s -wu_name %s -delay_bound %d" % (
            project.db_name, project.download_dir, project.upload_url,
            project.download_url, os.path.join(project.key_dir,'upload_private'),
            self.app.name, self.rsc_iops, self.rsc_fpops, self.rsc_disk,
            self.wu_template, self.result_template, self.redundancy, self.wu_template,
            self.delay_bound)

        for input_file in self.input_files:
            cmd += ' ' + input_file

        run_tool(cmd)

proxy_pid = 0
def start_proxy(code):
    global proxy_pid
    pid = os.fork()
    if not pid:
        os._exit(verbose_shell_call("./testproxy 8080 localhost:80 '$code' 2>testproxy.log"))

    verbose_sleep("Starting proxy server", 1)
    proxy_pid = pid
    # check if child process died
    (pid,status) = os.waitpid(pid, os.WNOHANG)
    if pid:
        fatal_error("testproxy failed")
def stop_proxy():
    global proxy_pid
    if proxy_pid:
        os.kill(2, proxy_pid)

def test_msg(msg):
    print "-- Testing", msg, '-'*(66-len(msg))

def test_done():
    global errors
    if sys.__dict__.get('last_traceback'):
        if sys.last_type == KeyboardInterrupt:
            errors += 0.1
            sys.stderr.write("\nTest canceled by user\n")
        else:
            errors += 1
            sys.stderr.write("\nException thrown - bug in test scripts?\n")
    if errors:
        verbose_echo(0, "ERRORS: %d" % errors)
        sys.exit(int(errors))
    else:
        verbose_echo(1, "Passed test!")
        if VERBOSE == 1:
            print
        sys.exit(0)

atexit.register(test_done)
