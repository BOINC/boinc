## $Id$

# the module MySQLdb can be installed on debian with "apt-get install python2.2-mysqldb"

# TODO: make things work if build_dir != src_dir

from version import *
from boinc_db import *
import os, sys, glob, time, shutil, re, random
import MySQLdb

class Options:
    pass

options = Options()
errors = Options()
errors.count = 0

options.have_init = False
options.install_method = None
options.echo_verbose = 1
options.is_test = False
options.client_bin_filename = CLIENT_BIN_FILENAME
options.drop_db_first = False

def init():
    if options.have_init: return
    options.have_init = True
    options.tty = os.isatty(1)
    options.echo_overwrite = options.tty and options.echo_verbose==1

    # VERBOSE: 0 = print nothing
    #          1 = print some (default
    #              if output is a tty, overwrite lines.
    #          2 = print all

    if options.install_method == 'copy':
        options.install_function = shutil.copy
    elif options.install_method == 'link' or options.install_method == 'hardlink':
        options.install_function = my_link
    elif options.install_method == 'symlink' or options.install_method == 'softlink':
        options.install_function = my_symlink
    else:
        fatal_error("Invalid install method: %s"%options.install_method)

prev_overwrite = False
def verbose_echo(level, line):
    global prev_overwrite
    if level == 0:
        if prev_overwrite:
            print
        print line
        prev_overwrite = False
    elif options.echo_verbose >= level:
        if options.echo_overwrite:
            print "\r                                                                               ",
            print "\r", line,
            sys.stdout.flush()
            prev_overwrite = True
        else:
            print line

def fatal_error(msg):
    errors.count += 1
    verbose_echo(0, "FATAL ERROR: "+msg)
    sys.exit(1)

def error(msg, fatal=0):
    if fatal: fatal_error(msg)
    errors.count += 1
    verbose_echo(0, "ERROR: "+msg)

def verbose_sleep(msg, wait):
    front = msg + ' [sleep '
    back = ']'
    for i in range(1,wait+1):
        verbose_echo(1, msg + ' [sleep ' + ('.'*i).ljust(wait) + ']')
        time.sleep(1)

def get_env_var(name, default = None):
    value = os.environ.get(name, default)
    if value == None:
        print "Environment variable %s not defined" % name
        sys.exit(1)
    return value

def shell_call(cmd, doexec=False, failok=False):
    if doexec:
        os.execl('/bin/sh', 'sh', '-c', cmd)
        error("Command failed: "+cmd, fatal=(not failok))
        os._exit(1)
    if os.system(cmd):
        error("Command failed: "+cmd, fatal=(not failok))
        return 1
    return 0

def verbose_shell_call(cmd, doexec=False, failok=False):
    verbose_echo(2, "   "+cmd)
    return shell_call(cmd, doexec, failok)

def destpath(src,dest):
    if dest.endswith('/'):
        return dest + os.path.basename(src)
    else:
        return dest

# my_symlink and my_link just add the filename to the exception object if one
# is raised - don't know why it's not already there
def my_symlink(src,dest):
    dest = destpath(src,dest)
    try:
        os.symlink(src,dest)
    except OSError, e:
        e.filename = dest
        raise

def my_link(src,dest):
    dest = destpath(src,dest)
    try:
        os.link(src,dest)
    except OSError, e:
        e.filename = dest
        raise

# install = options.install_function
def install(src, dest):
    options.install_function(src, dest)

def install_glob(glob_source, dest, failok=False):
    dest = os.path.join(dest, '') # append '/' if necessary
    for src in glob.glob(glob_source):
        if not os.path.isdir(src):
            install(src, dest)

def macro_substitute(macro, replacement, infile, outfile):
    open(outfile, 'w').write(open(infile).read().replace(macro, replacement))
def macro_substitute_inplace(macro, replacement, inoutfile):
    old = inoutfile + '.old'
    os.rename(inoutfile, old)
    macro_substitute(macro, replacement, old, inoutfile)

def check_program_exists(prog):
    if not os.path.isfile(prog):
        fatal_error("""
Executable not found: %s
Did you `make' yet?
""" % prog)
def check_core_client_executable():
    check_program_exists(builddir('client', CLIENT_BIN_FILENAME))
def check_app_executable(app):
    check_program_exists(builddir('apps', app))

def make_executable(name):
    os.chmod(name, 755)
def force_symlink(src, dest):
    if os.path.exists(dest):
        os.unlink(dest)
    my_symlink(src, dest)
def rmtree(dir):
    if os.path.exists(dir):
        shutil.rmtree(dir)

def _remove_trail(s, suffix):
    if s.endswith(suffix):
        return s[:-len(suffix)]
    else:
        return s

def _url_to_filename(url):
    return _remove_trail(url.replace('http://','').replace('/','_'),'_')
def account_file_name(url):
    return 'account_' + _url_to_filename(url) + '.xml'

def srcdir(*dirs):
    return apply(os.path.join,(TOP_SRC_DIR,)+dirs)

def builddir(*dirs):
    return apply(os.path.join,(TOP_BUILD_DIR,)+dirs)

def run_tool(cmd):
    verbose_shell_call(builddir('tools', cmd))

def _gen_key_p(private_key, public_key):
    shell_call("%s/crypt_prog -genkey 1024 %s %s >/dev/null" % (
        builddir('lib'),
        private_key,
        public_key))
def _gen_key(key):
    _gen_key_p(key+'_private', key+'_public')

def get_int(s):
    '''Convert a string to an int; return 0 on error.'''
    try: return int(sys.argv[1])
    except: return 0

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

def _check_vars(dict, **names):
    for key in names:
        value = names[key]
        if not key in dict:
            if value == None:
                raise SystemExit('error in test script: required parameter "%s" not specified'%key)
            dict[key] = value
    for key in dict:
        if not key in names:
            raise SystemExit('error in test script: extraneous parameter "%s" unknown'%key)

def db_query(db, query):
    db.query(query)
    result = db.use_result()
    return result and result.fetch_row(0,1)

def num_results(db, q=""):
    return db_query(db, "select count(*) from result "+q)[0]['count(*)']
def num_wus_left(db):
    return num_results(db, "where server_state=%d"%RESULT_SERVER_STATE_UNSENT)
def num_results_done(db):
    return num_results(db, "where server_state=%d"%RESULT_SERVER_STATE_OVER)

def query_yesno(str):
    '''Query user; default Yes'''
    verbose_echo(0,'')
    print str, "[Y/n] ",
    return not raw_input().strip().lower().startswith('n')

def query_noyes(str):
    '''Query user; default No'''
    verbose_echo(0,'')
    print str, "[y/N] ",
    return raw_input().strip().lower().startswith('y')

class Platform:
    def __init__(self, name, user_friendly_name=None):
        self.name = name
        self.user_friendly_name = user_friendly_name or name

class CoreVersion:
    def __init__(self):
        self.version = 1
        self.platform = Platform(PLATFORM)
        self.exec_dir = builddir('client')
        self.exec_name = options.client_bin_filename

class App:
    def __init__(self, name):
        assert(name)
        self.name = name

class AppVersion:
    def __init__(self, app, version = 1, exec_names=None):
        self.exec_names = []
        self.exec_dir = builddir('apps')
        self.exec_names = exec_names or [app.name]
        self.app = app
        self.version = 1
        self.platform = Platform(PLATFORM)

class Project:
    def __init__(self,
                 short_name, long_name, appname=None,
                 project_dir=None, master_url=None, cgi_url=None,
                 core_versions=None, key_dir=None,
                 apps=None, app_versions=None,
                 resource_share=None, redundancy=None):
        init()
        self.config_options = []
        self.config_daemons = []
        self.short_name     = short_name or 'test_'+appname
        self.long_name      = long_name or 'Project ' + self.short_name.replace('_',' ').capitalize()
        self.db_passwd      = ''
        self.shmem_key      = generate_shmem_key()
        self.resource_share = resource_share or 1
        self.redundancy     = redundancy or 2
        self.output_level   = 3

        self.master_url    = master_url or os.path.join(options.html_url , self.short_name , '')
        self.download_url  = os.path.join(self.master_url, 'download')
        self.cgi_url       = cgi_url or os.path.join(options.cgi_url, self.short_name)
        self.upload_url    = os.path.join(self.cgi_url     , 'file_upload_handler')
        self.scheduler_url = os.path.join(self.cgi_url     , 'cgi')
        self.project_dir   = project_dir or os.path.join(options.projects_dir     , self.short_name)
        self.download_dir  = os.path.join(self.project_dir , 'download')
        self.upload_dir    = os.path.join(self.project_dir , 'upload')
        self.key_dir       = key_dir or os.path.join(self.project_dir , 'keys')
        self.user_name     = options.user_name
        self.db_name       = self.user_name + '_' + self.short_name
        self.project_php_file                  = srcdir('html_user/project.inc.sample')
        self.project_specific_prefs_php_file   = srcdir('html_user/project_specific_prefs.inc.sample')

        self.core_versions = core_versions or [CoreVersion()]
        self.app_versions  = app_versions or [AppVersion(App(appname))]
        self.apps          = apps or unique(map(lambda av: av.app, self.app_versions))
        self.platforms     = [Platform(PLATFORM)]
        # convenience vars:
        self.app_version   = self.app_versions[0]
        self.app           = self.apps[0]

    def dir(self, *dirs):
        return apply(os.path.join,(self.project_dir,)+dirs)

    def keydir(self, *dirs):
        return apply(os.path.join,(self.key_dir,)+dirs)

    def run_db_script(self, script):
        shell_call('mysql %s < %s' % (self.db_name,srcdir('db', script)))

    def drop_db_if_exists(self):
        shell_call('echo "drop database if exists %s" | mysql' % self.db_name)

    def create_db(self):
        if options.drop_db_first:
            self.drop_db_if_exists()
        shell_call('echo "create database %s" | mysql' % self.db_name)

    def db_open(self):
        return MySQLdb.connect(db=self.db_name)

    def create_keys(self):
        if not os.path.exists(self.keydir()):
            os.mkdir(self.keydir())
        _gen_key(self.keydir('upload'))
        _gen_key(self.keydir('code_sign'))

    def query_create_keys(self):
        return query_yesno("Keys don't exist in %s; generate them?"%self.key_dir)

    def keys_exist(self):
        keys = ['upload_private', 'upload_public',
                'code_sign_private', 'code_sign_public' ]
        for key in keys:
            if not os.path.exists(self.keydir(key)): return False
        return True

    def install_project(self, scheduler_file = None):
        if os.path.exists(self.dir()):
            raise SystemExit('Project directory "%s" already exists; this would clobber it!'%self.dir())

        verbose_echo(1, "Setting up server: creating directories");
        # make the CGI writeable in case scheduler writes req/reply files
        # TODO: that is a security risk; don't do this in the future - write
        # req/reply files somewhere else
        map(lambda dir: os.mkdir(self.dir(dir)),
            [ '', 'cgi-bin', 'bin', 'upload', 'download', 'log',
              'html_ops', 'html_user', 'html_user/project_specific'])
        map(lambda dir: os.chmod(self.dir(dir), 0777),
            [ 'cgi-bin', 'upload', 'log' ])

        if not self.keys_exist():
            if self.query_create_keys():
                verbose_echo(1, "Setting up server files: generating keys");
                self.create_keys()

        # copy the user and administrative PHP files to the project dir,
        verbose_echo(1, "Setting up server files: copying html directories")

        install_glob(srcdir('html_user/*.php'), self.dir('html_user/'))
        install_glob(srcdir('html_user/*.inc'), self.dir('html_user/'))
        install_glob(srcdir('html_user/*.txt'), self.dir('html_user/'))
        install_glob(srcdir('html_ops/*.php'), self.dir('html_ops/'))
        install_glob(srcdir('html_ops/*.inc'), self.dir('html_ops/'))
        install(builddir('tools/country_select'), self.dir('html_user/'))
        install(self.project_php_file,
                self.dir('html_user', 'project_specific', 'project.inc'))
        install(self.project_specific_prefs_php_file,
                self.dir('html_user', 'project_specific', 'project_specific_prefs.inc'))

        my_symlink(self.download_dir, self.dir('html_user', 'download'))

        # Copy the sched server in the cgi directory with the cgi names given
        # source_dir/html_usr/schedulers.txt
        #

        verbose_echo(1, "Setting up server files: copying cgi programs");
        if scheduler_file:
            r = re.compile('<scheduler>([^<]+)</scheduler>', re.IGNORECASE)
            f = open(self.dir('html_user', scheduler_file))
            for line in f:
                # not sure if this is what the scheduler file is supposed to
                # mean
                match = r.search(line)
                if match:
                    cgi_name = match.group(1)
                    verbose_echo(2, "Setting up server files: copying " + cgi_name);
                    install(builddir('sched/cgi'), self.dir('cgi-bin', cgi_name,''))
            f.close()
        else:
            scheduler_file = 'schedulers.txt'
            f = open(self.dir('html_user', scheduler_file), 'w')
            print >>f, "<scheduler>" + self.scheduler_url, "</scheduler>"
            f.close()

        # copy all the backend programs
        map(lambda (s): install(builddir('sched',s), self.dir('cgi-bin',s)),
            [ 'cgi', 'file_upload_handler'])
        map(lambda (s): install(builddir('sched',s), self.dir('bin',s)),
            [ 'make_work',
              'feeder', 'timeout_check', 'validate_test',
              'file_deleter', 'assimilator' ])
        map(lambda (s): install(srcdir('sched',s), self.dir('bin',s)),
            [ 'start', 'stop', 'status',
              'boinc_config.py', 'grep_logs' ])

        verbose_echo(1, "Setting up database")
        self.create_db()
        map(self.run_db_script, [ 'schema.sql' ])

        db = self.db_open()
        db.query("insert into project(short_name, long_name) values('%s', '%s')" %(
            self.short_name, self.long_name));

        verbose_echo(1, "Setting up database: adding %d apps(s)" % len(self.apps))
        for app in self.apps:
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
                check_app_executable(exec_name)
                cmd += ' ' + exec_name
            run_tool(cmd)

        verbose_echo(1, "Setting up server files: writing config files");

        config = map_xml(self,
                         [ 'db_name', 'db_passwd', 'shmem_key',
                           'key_dir', 'download_url', 'download_dir',
                           'upload_url', 'upload_dir', 'project_dir', 'user_name',
                           'cgi_url',
                           'output_level' ])
        self.config_options = config.split('\n')
        self.write_config()

        # edit "index.php" in the user HTML directory to have the right file
        # as the source for scheduler_urls; default is schedulers.txt

        macro_substitute_inplace('FILE_NAME', scheduler_file,
                                 self.dir('html_user', 'index.php'))

        # create symbolic links to the CGI and HTML directories
        verbose_echo(1, "Setting up server files: linking cgi programs")
        if options.__dict__.get('cgi_dir'):
            force_symlink(self.dir('cgi-bin'), os.path.join(options.cgi_dir, self.short_name))
        if options.__dict__.get('html_dir'):
            force_symlink(self.dir('html_user'), os.path.join(options.html_dir, self.short_name))
            force_symlink(self.dir('html_ops'), os.path.join(options.html_dir, self.short_name+'_admin'))

        # show the URLs for user and admin sites
        # admin_url = os.path.join("html_user", self.short_name+'_admin/')

        # verbose_echo(2, "Master URL: " + self.master_url)
        # verbose_echo(2, "Admin URL:  " + admin_url)

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

    def _run_sched_prog(self, prog, args='', logfile=None):
        verbose_shell_call("cd %s && ./%s %s >> %s.log 2>&1" %
                           (self.dir('bin'), prog, args, (logfile or prog)))

    def start_servers(self):
        self.started = True
        self._run_sched_prog('start', '-v --enable')
        verbose_sleep("Starting servers for project '%s'" % self.short_name, 1)

    def _build_sched_commandlines(self, progname, kwargs):
        '''Given a KWARGS dictionary build a list of command lines string depending on the program.'''
        each_app = False
        if progname == 'feeder':
            _check_vars(kwargs)
        elif progname == 'timeout_check':
            _check_vars(kwargs, app=self.app.name, nerror=5, ndet=5, nredundancy=5)
        elif progname == 'make_work':
            work = kwargs.get('work', self.work)
            _check_vars(kwargs, cushion=None, redundancy=self.redundancy,
                        result_template=os.path.realpath(work.result_template),
                        wu_name=work.wu_template)
        elif progname == 'validate_test':
            _check_vars(kwargs, quorum=self.redundancy)
            each_app = True
        elif progname == 'file_deleter':
            _check_vars(kwargs)
        elif progname == 'assimilator':
            _check_vars(kwargs)
            each_app = True
        else:
            raise SystemExit("test script error: invalid progname '%s'"%progname)
        cmdline = ' '.join(map(lambda k: '-%s %s'%(k,kwargs[k]), kwargs.keys()))
        if each_app:
            return map(lambda av: '-app %s %s'%(av.app.name,cmdline), self.app_versions)
        else:
            return [cmdline]

    def sched_run(self, prog, **kwargs):
        for cmdline in self._build_sched_commandlines(prog, kwargs):
            self._run_sched_prog(prog, '-d 3 -one_pass '+cmdline)
    def sched_install(self, prog, **kwargs):
        for cmdline in self._build_sched_commandlines(prog, kwargs):
            self.config_daemons.append("%s -d 3 %s" %(prog, cmdline))
            self.write_config()
    def sched_uninstall(self, prog):
        self.config_daemons = filter(lambda l: l.find(prog)==-1, self.config_daemons)
        self.write_config()

    def start_stripcharts(self):
        map(lambda l: self.copy(os.path.join('stripchart', l), 'cgi-bin/'),
            [ 'stripchart.cgi', 'stripchart', 'stripchart.cnf',
              'looper', 'db_looper', 'datafiles', 'get_load', 'dir_size' ])
        macro_substitute('BOINC_DB_NAME', self.db_name, srcdir('stripchart/samples/db_count'),
                         self.dir('bin/db_count'))
        make_executable(self.dir('bin/db_count'))

        self._run_sched_prog('looper'    , 'get_load 1'                            , 'get_load')
        self._run_sched_prog('db_looper' , '"result" 1'                            , 'count_results')
        self._run_sched_prog('db_looper' , '"workunit where assimilate_state=2" 1' , 'assimilated_wus')
        self._run_sched_prog('looper'    , '"dir_size ../download" 1'              , 'download_size')
        self._run_sched_prog('looper'    , '"dir_size ../upload" 1'                , 'upload_size')

    def stop(self):
        verbose_echo(1,"Stopping server(s) for project '%s'"%self.short_name)
        self._run_sched_prog('start', '-v --disable')
        self.started = False

    def maybe_stop(self):
        if self.started: self.stop()

    def write_config(self):
        f = open(self.dir('config.xml'), 'w')
        print >>f, '<boinc>'
        print >>f, '  <config>'
        for line in self.config_options:
            print >>f, "     ", line
        print >>f, '  </config>'
        print >>f, '  <daemons>'
        for daemon in self.config_daemons:
            print >>f, "       <daemon><cmd>%s</cmd></daemon>"%daemon
        print >>f, '  </daemons>'
        print >>f, '</boinc>'
