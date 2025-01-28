# module for setting up a new project (either a real project or a test project
# see tools/makeproject, test/testbase.py).

# TODO: make sure things work if build_dir != src_dir

from __future__ import print_function
import boinc_path_config
from Boinc import database, db_mid, configxml, tools
from Boinc.boinc_db import *
import os, sys, glob, time, shutil, re, random

class Options:
    pass

options = Options()
errors = Options()
errors.count = 0

options.have_init = False
options.install_method = None
options.is_test = False
options.drop_db_first = False

def init():
    if options.have_init: return
    options.have_init = True

    if options.install_method == 'copy':
        options.install_function = shutil.copy
    elif options.install_method == 'link' or options.install_method == 'hardlink':
        options.install_function = my_link
    elif options.install_method == 'symlink' or options.install_method == 'softlink':
        options.install_function = my_symlink
    else:
        fatal_error("Invalid install method: %s"%options.install_method)

def verbose_echo(level, line):
    print(line)
    sys.stdout.flush()

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
    if value is None:
        print("Environment variable %s not defined" % name)
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
    except OSError as e:
        e.filename = src + ' -> ' + dest
        raise

def my_link(src,dest):
    dest = destpath(src,dest)
    try:
        os.link(src,dest)
    except OSError as e:
        e.filename = src + ' -> ' + dest
        raise

# install = options.install_function
def install(src, dest, unless_exists=False):
    if unless_exists and os.path.exists(dest):
        return
    try:
        options.install_function(src, dest)
    except:
        print('failed to copy ' + src + ' to ' + dest)
        return


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
    check_program_exists(builddir('client', version.CLIENT_BIN_FILENAME))
def check_app_executable(app):
    check_program_exists(builddir('apps', app))

def make_executable(name):
    os.chmod(name, 755)
def force_symlink(src, dest):
    if os.path.exists(dest):
        os.unlink(dest)
    my_symlink(src, dest)
def rmtree(dir):
    # if os.path.exists(dir):
    #     shutil.rmtree(dir)
    if not dir or dir == '/' or dir == '.' or ' ' in dir:
        raise Exception
    os.system("rm -rf %s"%dir)

def _remove_trail(s, suffix):
    if s.endswith(suffix):
        return s[:-len(suffix)]
    else:
        return s

def _url_to_filename(url):
    s=""
    for c in url.replace('http://',''):
        if (c.isalnum()):
            s += c
        else:
            s += '_'
    return _remove_trail(s,'_')

def account_file_name(url):
    return 'account_' + _url_to_filename(url) + '.xml'

def srcdir(location):
    return os.path.join(options.srcdir, location)

def builddir(location):
    return os.path.join(boinc_path_config.TOP_BUILD_DIR, location)

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
    try: return int(s)
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
            if value is None:
                raise SystemExit('error in test script: required parameter "%s" not specified'%key)
            dict[key] = value
    for key in dict:
        if not key in names:
            raise SystemExit('error in test script: extraneous parameter "%s" unknown'%key)

# CAN REMOVE THE FOLLOWING 8 FNS??

# def db_query(db, query):
#     db.query(query)
#     result = db.use_result()
#     return result and result.fetch_row(0,1)

def num_results():
    return database.Results.count()
def num_results_unsent():
    return database.Results.count(server_state = RESULT_SERVER_STATE_UNSENT)
def num_results_in_progress():
    return database.Results.count(server_state = RESULT_SERVER_STATE_IN_PROGRESS)
def num_results_over():
    return database.Results.count(server_state = RESULT_SERVER_STATE_OVER)
def num_wus():
    return database.Workunits.count()
def num_wus_assimilated():
    return database.Workunits.count(assimilate_state = ASSIMILATE_DONE)
def num_wus_to_transition():
    return database.Workunits.count(_extra_params = ['transition_time<%d'%(time.time()+30*86400)])

def build_command_line(cmd, kwargs):
    for (key, value) in kwargs:
        cmd += " -%s '%s'" %(key,value)
    return cmd

def create_project_dirs(dest_dir):
    def mkdir2(d):
        try:
            os.makedirs(d)
        except OSError as e:
            if not os.path.isdir(d):
                raise SystemExit(e)

    directories = ('',
                   'cgi-bin',
                   'bin',
                   'py',
                   'py/Boinc',
                   'templates',
                   'upload',
                   'download',
                   'apps',
                   'html',
                   'html/cache',
                   'html/inc',
                   'html/inc/password_compat',
                   'html/inc/random_compat',
                   'html/inc/ReCaptcha',
                   'html/inc/ReCaptcha/RequestMethod',
                   'html/languages',
                   'html/languages/compiled',
                   'html/languages/translations',
                   'html/languages/project_specific_translations',
                   'html/ops',
                   'html/ops/ffmail',
                   'html/ops/mass_email',
                   'html/ops/remind_email',
                   'html/project',
                   'html/stats',
                   'html/user',
                   'html/user/img',
                   'html/user_profile',
                   'html/user_profile/images'
    )
    [ mkdir2(os.path.join(dest_dir, x)) for x in directories ]

    # For all directories that apache will put files in,
    # make them group-writeable and setGID.
    # Assuming that the "apache" user belongs to our primary group,
    # any files or dirs created by apache will be owned by
    # our primary group (not Apache's).
    #
    directories =  [
        'upload',
        'html/cache',
        'html/inc',
        'html/languages',
        'html/languages/compiled',
        'html/user_profile/images',
    ]
    for d in directories:
        os.chmod(os.path.join(dest_dir, d), 0o2770)

def install_boinc_files(dest_dir, install_web_files, install_server_files):
    """Copy files from source dir to project dir.
        Used by the upgrade script, so don't copy sample files to real name."""

    def dest(*dirs):
        location = dest_dir
        for d in dirs:
            location = os.path.join(location, d )
        return location

    create_project_dirs(dest_dir)

    # copy html/ops files in all cases.
    # The critical one is db_update.php,
    # which is needed even for a server_only upgrade
    install_glob(srcdir('html/ops/*.php'), dest('html/ops/'))

    if install_web_files:
        install_glob(srcdir('html/inc/*.inc'), dest('html/inc/'))
        install_glob(srcdir('html/inc/*.php'), dest('html/inc/'))
        install_glob(srcdir('html/inc/password_compat/*.inc'), dest('html/inc/password_compat/'))
        install_glob(srcdir('html/inc/random_compat/*.inc'), dest('html/inc/random_compat/'))
        install_glob(srcdir('html/inc/ReCaptcha/*.php'), dest('html/inc/ReCaptcha/'))
        install_glob(srcdir('html/inc/ReCaptcha/RequestMethod/*.php'), dest('html/inc/ReCaptcha/RequestMethod'))
        install_glob(srcdir('html/inc/*.dat'), dest('html/inc/'))
        install_glob(srcdir('html/ops/*.css'), dest('html/ops/'))
        install_glob(srcdir('html/ops/ffmail/sample*'), dest('html/ops/ffmail/'))
        install_glob(srcdir('html/ops/mass_email/sample*'), dest('html/ops/mass_email/'))
        install_glob(srcdir('html/ops/remind_email/sample*'), dest('html/ops/remind_email/'))
        install_glob(srcdir('html/user/*.php'), dest('html/user/'))
        install_glob(srcdir('html/user/*.inc'), dest('html/user/'))
        install_glob(srcdir('html/user/*.css'), dest('html/user/'))
        install_glob(srcdir('html/user/*.txt'), dest('html/user/'))
        install_glob(srcdir('html/user/*.js'), dest('html/user/'))
        install_glob(srcdir('html/user/*.png'), dest('html/user/img'))
        install_glob(srcdir('html/user/*.gif'), dest('html/user/img'))
        install_glob(srcdir('html/user/img/*.*'), dest('html/user/img'))
        if not os.path.exists(dest('html/user/motd.php')):
            shutil.copy(srcdir('html/user/sample_motd.php'), dest('html/user/motd.php'))
        os.system("rm -f "+dest('html/languages/translations/*'))
        install_glob(srcdir('html/languages/translations/*.po'), dest('html/languages/translations/'))

    # copy Python stuff
    install(srcdir('sched/start' ), dest('bin/start' ))
    force_symlink(dest('bin', 'start'), dest('bin', 'stop'))
    force_symlink(dest('bin', 'start'), dest('bin', 'status'))
    python_files = [
        '__init__.py',
        'add_util.py',
        'boinc_db.py',
        'boinc_project_path.py',
        'boincxml.py',
        'configxml.py',
        'database.py',
        'db_base.py',
        'db_mid.py',
        'projectxml.py',
        'sched_messages.py',
        'tools.py',
        'util.py'
    ]
    for s in python_files:
        install(srcdir("py/Boinc/" + s), dest('py/Boinc', s))

    content = '''
# Generated by make_project
import sys, os
sys.path.insert(0, os.path.join('{dest_dir}', 'py'))
'''.format(dest_dir=dest_dir)
    f = open(dest('bin', 'boinc_path_config.py'), "w")
    f.write(content)
    f.close()

    if not install_server_files:
        return

    # copy backend (C++) programs;
    # rename current web daemons in case they're in use

    if os.path.isfile(dest('cgi-bin', 'cgi')):
        os.rename(dest('cgi-bin', 'cgi'), dest('cgi-bin', 'cgi.old'))
    if os.path.isfile(dest('cgi-bin', 'fcgi')):
        os.rename(dest('cgi-bin', 'fcgi'), dest('cgi-bin', 'fcgi.old'))
        install(builddir('sched','fcgi'), dest('cgi-bin','fcgi'))
    if os.path.isfile(dest('cgi-bin', 'file_upload_handler')):
        os.rename(dest('cgi-bin', 'file_upload_handler'), dest('cgi-bin', 'file_upload_handler.old'))
    cgi_script = [ 'cgi', 'file_upload_handler']
    for f in cgi_script:
        install(builddir('sched/' + f), dest('cgi-bin',f))
    command = [
            'adjust_user_priority',
            'antique_file_deleter',
            'assimilator.py',
            'census',
            'db_dump',
            'db_purge',
            'delete_file',
            'feeder',
            'file_deleter',
            'get_file',
            'make_work',
            'pshelper',
            'put_file',
            'pymw_assimilator.py',
            'sample_assimilator',
            'sample_bitwise_validator',
            'sample_dummy_assimilator',
            'sample_substr_validator',
            'sample_trivial_validator',
            'sample_work_generator',
            'script_assimilator',
            'script_validator',
            'show_shmem',
            'single_job_assimilator',
            'size_regulator',
            'transitioner',
            'transitioner_catchup.php',
            'trickle_credit',
            'trickle_deadline',
            'trickle_echo',
            'update_stats',
            'wu_check',
        ]
    for f in command:
        install(builddir('sched/' + f), dest('bin',f))
    command = [ 'vda', 'vdad' ]
    for f in command:
        install(builddir('vda/' + f), dest('bin',f))
    command = [
            'appmgr',
            'boinc_submit',
            'cancel_jobs',
            'create_work',
            'dbcheck_files_exist',
            'dir_hier_move',
            'dir_hier_path',
            'grep_logs',
            'manage_privileges',
            'parse_config',
            'query_job',
            'run_in_ops',
            'sample_assimilate.py',
            'sign_executable',
            'stage_file',
            'stage_file_native',
            'submit_batch',
            'submit_buda',
            'submit_job',
            'update_versions',
            'xadd',
        ]
    for f in command:
        install(builddir('tools/' + f), dest('bin',f))
    install(srcdir('lib/crypt_prog'), dest('bin','crypt_prog'))
    install(srcdir('sched/db_dump_spec.xml' ), dest('','db_dump_spec.xml' ))

class Project:
    def __init__(self,
                 short_name, long_name,
                 cgi_url,
                 project_dir=None, key_dir=None,
                 master_url=None,
                 db_name=None,
                 host=None,
                 web_only=False,
                 no_db=False,
                 production=False
                 ):
        init()

        self.production     = production
        self.web_only       = web_only
        self.no_db          = no_db
        self.short_name     = short_name
        self.long_name      = long_name or 'Project ' + self.short_name.replace('_',' ').capitalize()

        self.project_dir   = project_dir or os.path.join(options.projects_dir, self.short_name)

        self.config = configxml.ConfigFile(self.dest('config.xml')).init_empty()
        config = self.config.config

        # this is where default project config is defined

        config.long_name = self.long_name
        config.db_user = options.db_user
        config.db_name = db_name or options.user_name + '_' + self.short_name
        config.db_passwd = options.db_passwd
        config.db_host = options.db_host
        config.shmem_key = generate_shmem_key()
        config.uldl_dir_fanout = 1024
        config.host = host
        config.min_sendwork_interval = 0
        config.max_wus_to_send = 50
        config.daily_result_quota = 500
        config.disable_account_creation = 0
        config.disable_account_creation_rpc = 0
        config.account_creation_rpc_require_consent = 0
        config.disable_web_account_creation = 0
        config.enable_login_mustagree_termsofuse = 0
        config.enable_privacy_by_default = 0
        config.show_results = 1
        config.cache_md5_info = 1
        config.sched_debug_level = 3
        config.fuh_debug_level = 3
        config.one_result_per_user_per_wu = 0
        config.send_result_abort = 1
        config.dont_generate_upload_certificates = 1
        config.ignore_upload_certificates = 1
        config.enable_delete_account = 0
        if web_only:
            config.no_computing = 1

        config.master_url    = master_url or os.path.join(options.html_url , self.short_name , '')
        config.download_url  = os.path.join(config.master_url, 'download')
        config.upload_url    = os.path.join(cgi_url     , 'file_upload_handler')
        config.download_dir  = os.path.join(self.project_dir , 'download')
        config.upload_dir    = os.path.join(self.project_dir , 'upload')
        config.key_dir       = key_dir or os.path.join(self.project_dir , 'keys')
        config.app_dir       = os.path.join(self.project_dir, 'apps')
        config.log_dir       = self.project_dir+'log_'+config.host
        if production:
            config.min_sendwork_interval = 6
        self.scheduler_url = os.path.join(cgi_url     , 'cgi')

    def dest(self, *dirs):
        location = self.project_dir
        for x in dirs:
            location = os.path.join(location, x)
        return location

    def keydir(self, location):
            return os.path.join(self.config.config.key_dir, location)

    def logdir(self):
        return os.path.join(self.project_dir, "log_"+self.config.config.host)

    def create_keys(self):
        if not os.path.exists(self.config.config.key_dir):
            os.mkdir(self.config.config.key_dir)
        _gen_key(self.keydir('upload'))
        _gen_key(self.keydir('code_sign'))

    def create_logdir(self):
        os.mkdir(self.logdir())
        os.chmod(self.logdir(), 0o2770)

    def keys_exist(self):
        keys = ['upload_private', 'upload_public',
                'code_sign_private', 'code_sign_public' ]
        for key in keys:
            if not os.path.exists(self.keydir(key)): return False
        return True

    # create new project.  Called only from make_project
    def install_project(self):
        if os.path.exists(self.dest()):
            raise SystemExit('Project directory "%s" already exists; this would clobber it!'%self.dest())

        verbose_echo(1, "Creating directories")

        create_project_dirs(self.project_dir)

        if not self.web_only:
            if not self.keys_exist():
                verbose_echo(1, "Generating encryption keys")
                self.create_keys()

        # copy the user and administrative PHP files to the project dir,
        verbose_echo(1, "Copying files")

        # Create the project log directory
        self.create_logdir()

        install_boinc_files(self.dest(), True, not self.web_only)

        # copy sample web files to final names
        install(srcdir('html/user/sample_index.php'),
            self.dest('html/user/index.php'))
        install(srcdir('html/user/sample_bootstrap.min.css'),
            self.dest('html/user/bootstrap.min.css'))
        install(srcdir('html/user/sample_bootstrap.min.js'),
            self.dest('html/user/bootstrap.min.js'))
        install(srcdir('html/user/sample_jquery.min.js'),
            self.dest('html/user/jquery.min.js'))
        install(srcdir('html/project.sample/project.inc'),
            self.dest('html/project/project.inc'))
        install(srcdir('html/project.sample/project_specific_prefs.inc'),
            self.dest('html/project/project_specific_prefs.inc'))
        install(srcdir('html/project.sample/cache_parameters.inc'),
            self.dest('html/project/cache_parameters.inc'))
        install(srcdir('tools/project.xml'), self.dest('project.xml'))
        install(srcdir('tools/gui_urls.xml'), self.dest('gui_urls.xml'))
        if not self.production:
            install(srcdir('test/uc_result'), self.dest('templates/uc_result'))
            install(srcdir('test/uc_wu_nodelete'), self.dest('templates/uc_wu'))

        content = '''
<!-- <scheduler>{url}</scheduler> -->
<link rel="boinc_scheduler" href="{url}">
        '''.format(url=self.scheduler_url.strip())
        f = open(self.dest('html/user', 'schedulers.txt'), 'w')
        f.write(content)
        f.close()

        if self.no_db:
            verbose_echo(1, "Not setting up database (--no_db was specified)")
        else:
            verbose_echo(1, "Setting up database")
            database.create_database(
                srcdir = options.srcdir,
                config = self.config.config,
                drop_first = options.drop_db_first
                )

        verbose_echo(1, "Writing config files")
        self.config.write()

        # create symbolic links to the CGI and HTML directories
        verbose_echo(1, "Linking CGI programs")
        if options.__dict__.get('cgi_dir'):
            force_symlink(self.dest('cgi-bin'), os.path.join(options.cgi_dir, self.short_name))
        if options.__dict__.get('html_dir'):
            force_symlink(self.dest('html/user'), os.path.join(options.html_dir, self.short_name))
            force_symlink(self.dest('html/ops'), os.path.join(options.html_dir, self.short_name+'_admin'))

    def http_password(self, user, password):
        'Adds http password protection to the html/ops directory'
        passwd_file = self.dest('html/ops', '.htpassword')
        content = '''
AuthName '{long_name} Administration'
AuthType Basic
AuthUserFile {passwd_file}
require valid-user
'''.format(long_name=self.long_name, passwd_file=passwd_file)
        f = open(self.dest('html/ops', '.htaccess'), 'w')
        f.write(content)
        f.close()
        shell_call("htpassword -bc %s %s %s" % (passwd_file, user, password))

    def _run_sched_prog(self, prog, args='', logfile=None):
        verbose_shell_call("cd %s && ./%s %s >> %s.log 2>&1" %
                           (self.dest('bin'), prog, args, (logfile or prog)))

    def start_servers(self):
        self.started = True
        self._run_sched_prog('start', '-v --enable')
        verbose_sleep("Starting servers for project '%s'" % self.short_name, 1)

    def _build_sched_commandlines(self, progname, kwargs):
        '''Given a KWARGS dictionary build a list of command lines string depending on the program.'''
        each_app = False
        if progname == 'feeder':
            _check_vars(kwargs)
        elif progname == 'transitioner':
            _check_vars(kwargs)
        elif progname == 'make_work':
            work = kwargs.get('work', self.work)
            _check_vars(kwargs, cushion=30, max_wus=0, wu_name=work.wu_template)
        elif progname == 'sample_bitwise_validator':
            _check_vars(kwargs)
            each_app = True
        elif progname == 'file_deleter':
            _check_vars(kwargs)
        elif progname == 'antique_file_deleter':
            _check_vars(kwargs)
        elif progname == 'sample_dummy_assimilator':
            _check_vars(kwargs)
            each_app = True
        else:
            raise SystemExit("test script error: invalid progname '%s'"%progname)
        cmdline = build_command_line('', kwargs)
        if each_app:
            return [ '-app %s %s'%(av.app.name,cmdline) for av in self.app_versions ]
        else:
            return [cmdline]

    def sched_run(self, prog, **kwargs):
        for cmdline in self._build_sched_commandlines(prog, kwargs):
            self._run_sched_prog(prog, '-d 3 -one_pass '+cmdline)
    def sched_install(self, prog, **kwargs):
        for cmdline in self._build_sched_commandlines(prog, kwargs):
            self.config.daemons.make_node_and_append("daemon").cmd = "%s -d 3 %s" %(prog, cmdline)
        self.config.write()
    # def sched_uninstall(self, prog):
    #     self.config_daemons = XXX filter(lambda l: l.find(prog)==-1, self.config_daemons)
    #     self.config.write()

    def start_stripcharts(self):
        cgi_bin = [ 'stripchart.cgi', 'stripchart', 'stripchart.cnf',
            'looper', 'db_looper', 'datafiles', 'get_load', 'dir_size' ]
        for f in cgi_bin:
            self.copy(os.path.join('stripchart', f), 'cgi-bin/')

        macro_substitute('BOINC_DB_NAME', self.db_name, srcdir('stripchart/samples/db_count'),
                         self.dest('bin/db_count'))
        make_executable(self.dest('bin/db_count'))

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

def query_noyes(str):
    verbose_echo(0,'')
    return tools.query_noyes(str)

def query_yesno(str):
    verbose_echo(0,'')
    return tools.query_yesno(str)
