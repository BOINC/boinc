## $Id$

# module for setting up a new project (either a real project or a test project
# see tools/makeproject, test/testbase.py).

# TODO: make sure things work if build_dir != src_dir

import boinc_path_config
from Boinc import database, db_mid, configxml, tools
from Boinc.boinc_db import *
import os, sys, glob, time, shutil, re, random, socket

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
    print line
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
        e.filename = src + ' -> ' + dest
        raise

def my_link(src,dest):
    dest = destpath(src,dest)
    try:
        os.link(src,dest)
    except OSError, e:
        e.filename = src + ' -> ' + dest
        raise

# install = options.install_function
def install(src, dest, unless_exists=False):
    if unless_exists and os.path.exists(dest):
        return
    try:
        options.install_function(src, dest)
    except:
        print 'failed to copy ' + src + ' to ' + dest
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

def srcdir(*dirs):
    return apply(os.path.join,(options.srcdir,)+dirs)

def builddir(*dirs):
    return apply(os.path.join,(boinc_path_config.TOP_BUILD_DIR,)+dirs)

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
            if value == None:
                raise SystemExit('error in test script: required parameter "%s" not specified'%key)
            dict[key] = value
    for key in dict:
        if not key in names:
            raise SystemExit('error in test script: extraneous parameter "%s" unknown'%key)

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

def build_command_line(cmd, **kwargs):
    for (key, value) in kwargs.items():
        cmd += " -%s '%s'" %(key,value)
    return cmd

def create_project_dirs(dest_dir):
    def dir(*d):
        return apply(os.path.join,(dest_dir,)+d)
    def mkdir2(d):
        try:
            os.mkdir(d);
        except:
            pass
    map(lambda d: mkdir2(dir(d)),
        [   '',
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
            'html/languages',
            'html/languages/compiled',
            'html/languages/translations',
            'html/languages/project_specific_translations',
            'html/ops',
            'html/ops/ffmail',
            'html/ops/mass_email',
            'html/ops/remind_email',
            'html/ops',
            'html/project',
            'html/stats',
            'html/user',
            'html/user/img',
            'html/user_profile',
            'html/user_profile/images'
        ])

    # For all directories that apache will put files in,
    # make them group-writeable and setGID.
    # Assuming that the "apache" user belongs to our primary group,
    # any files or dirs created by apache will be owned by
    # our primary group (not Apache's).
    #
    map(lambda d: os.chmod(dir(d), 02770),
        [
            'upload',
            'html/cache',
            'html/inc',
            'html/languages',
            'html/languages/compiled',
            'html/user_profile/images'
        ])

def install_boinc_files(dest_dir, install_web_files, install_server_files):
    """Copy files from source dir to project dir.
        Used by the upgrade script, so don't copy sample files to real name."""

    def dir(*dirs):
        return apply(os.path.join,(dest_dir,)+dirs)

    create_project_dirs(dest_dir);

    # make a symbolic link from html/user/user_profile to html/user_profile
    try:
        my_symlink(dir('html/user_profile'), dir('html/user/user_profile'));
    except:
        pass

    # copy html/ops files in all cases.
    # The critical one is db_update.php,
    # which is needed even for a server_only upgrade

    install_glob(srcdir('html/ops/*.php'), dir('html/ops/'))
    install_glob(srcdir('html/ops/*.inc'), dir('html/ops/'))

    if install_web_files:
        install_glob(srcdir('html/inc/*.inc'), dir('html/inc/'))
        install_glob(srcdir('html/inc/*.php'), dir('html/inc/'))
        install_glob(srcdir('html/inc/*.dat'), dir('html/inc/'))
        install_glob(srcdir('html/ops/ffmail/sample*'), dir('html/ops/ffmail/'))
        install_glob(srcdir('html/ops/mass_email/sample*'), dir('html/ops/mass_email/'))
        install_glob(srcdir('html/ops/remind_email/sample*'), dir('html/ops/remind_email/'))
        install_glob(srcdir('html/user/*.php'), dir('html/user/'))
        install_glob(srcdir('html/user/*.inc'), dir('html/user/'))
        install_glob(srcdir('html/user/*.css'), dir('html/user/'))
        install_glob(srcdir('html/user/*.txt'), dir('html/user/'))
        install_glob(srcdir('html/user/*.js'), dir('html/user/'))
        install_glob(srcdir('html/user/*.png'), dir('html/user/img'))
        install_glob(srcdir('html/user/*.gif'), dir('html/user/img'))
        install_glob(srcdir('html/user/img/*.*'), dir('html/user/img'))
        if not os.path.exists(dir('html/user/motd.php')):
            shutil.copy(srcdir('html/user/sample_motd.php'), dir('html/user/motd.php'))
        os.system("rm -f "+dir('html/languages/translations/*'))
        install_glob(srcdir('html/languages/translations/*.po'), dir('html/languages/translations/'))

    # copy Python stuff
    map(lambda (s): install(srcdir('sched',s), dir('bin',s)),
        [ 'start' ])
    force_symlink(dir('bin', 'start'), dir('bin', 'stop'))
    force_symlink(dir('bin', 'start'), dir('bin', 'status'))
    map(lambda (s): install(srcdir('py/Boinc',s), dir('py/Boinc',s)),
        [
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
        ])
    print >>open(dir('bin', 'boinc_path_config.py'), 'w'), '''
# Generated by make_project
import sys, os
sys.path.insert(0, os.path.join('%s', 'py'))
''' % dest_dir

    if not install_server_files:
        return

    # copy backend (C++) programs;
    # rename current web daemons in case they're in use

    if os.path.isfile(dir('cgi-bin', 'cgi')):
        os.rename(dir('cgi-bin', 'cgi'), dir('cgi-bin', 'cgi.old'))
    if os.path.isfile(dir('cgi-bin', 'fcgi')):
        os.rename(dir('cgi-bin', 'fcgi'), dir('cgi-bin', 'fcgi.old'))
        map(lambda (s): install(builddir('sched',s), dir('cgi-bin',s)),
            [ 'fcgi'])
    if os.path.isfile(dir('cgi-bin', 'file_upload_handler')):
        os.rename(dir('cgi-bin', 'file_upload_handler'), dir('cgi-bin', 'file_upload_handler.old'))

    map(lambda (s): install(builddir('sched',s), dir('cgi-bin',s)),
        [ 'cgi', 'file_upload_handler'])
    map(lambda (s): install(builddir('sched',s), dir('bin',s)),
        [
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
            'put_file',
            'pymw_assimilator.py',
            'sample_assimilator',
            'sample_bitwise_validator',
            'sample_dummy_assimilator',
            'sample_trivial_validator',
            'sample_work_generator',
            'show_shmem',
            'single_job_assimilator', 
            'transitioner',
            'transitioner_catchup.php',
            'update_stats'
        ])
    map(lambda (s): install(builddir('vda',s), dir('bin',s)),
        [ 'vda', 'vdad' ])
    map(lambda (s): install(srcdir('tools',s), dir('bin',s)),
        [
            'appmgr',
            'boinc_submit',
            'cancel_jobs',
            'create_work',
            'dbcheck_files_exist',
            'db_query',
            'demo_query',
            'demo_submit',
            'dir_hier_move',
            'dir_hier_path',
            'grep_logs',
            'manage_privileges',
            'parse_config',
            'run_in_ops',
            'sign_executable',
            'stage_file',
            'update_versions',
            'watch_tcp',
            'xadd'
        ])
    map(lambda (s): install(srcdir('lib',s), dir('bin',s)),
        [ 'crypt_prog' ])
    map(lambda (s): install(srcdir('sched',s), dir('',s)),
        [ 'db_dump_spec.xml' ])


class Project:
    def __init__(self,
                 short_name, long_name,
                 project_dir=None, key_dir=None,
                 master_url=None, cgi_url=None,
                 db_name=None,
                 web_only=False,
                 production=False
                 ):
        init()

        self.production     = production
        self.web_only       = web_only
        self.short_name     = short_name
        self.long_name      = long_name or 'Project ' + self.short_name.replace('_',' ').capitalize()

        self.project_dir   = project_dir or os.path.join(options.projects_dir, self.short_name)

        self.config = configxml.ConfigFile(self.dir('config.xml')).init_empty()
        config = self.config.config

        # this is where default project config is defined

        config.long_name = self.long_name
        config.db_user = options.db_user
        config.db_name = db_name or options.user_name + '_' + self.short_name
        config.db_passwd = options.db_passwd
        config.db_host = options.db_host
        config.shmem_key = generate_shmem_key()
        config.uldl_dir_fanout = 1024
        local_host = socket.gethostname()
        config.host = local_host.split('.')[0]
        config.min_sendwork_interval = 0
        config.max_wus_to_send = 50
        config.daily_result_quota = 500
        config.disable_account_creation = 0
        config.show_results = 1
        config.sched_debug_level = 3
        config.fuh_debug_level = 3
        config.one_result_per_user_per_wu = 0
        config.send_result_abort = 1
        config.dont_generate_upload_certificates = 1
        config.ignore_upload_certificates = 1
        if web_only:
            config.no_computing = 1

        config.master_url    = master_url or os.path.join(options.html_url , self.short_name , '')
        config.download_url  = os.path.join(config.master_url, 'download')
        config.cgi_url       = cgi_url or os.path.join(options.cgi_url, self.short_name)
        config.upload_url    = os.path.join(config.cgi_url     , 'file_upload_handler')
        config.download_dir  = os.path.join(self.project_dir , 'download')
        config.upload_dir    = os.path.join(self.project_dir , 'upload')
        config.key_dir       = key_dir or os.path.join(self.project_dir , 'keys')
        config.app_dir       = os.path.join(self.project_dir, 'apps')
        config.log_dir       = self.project_dir+'log_'+config.host
        if production:
            config.min_sendwork_interval = 6
        self.scheduler_url = os.path.join(config.cgi_url     , 'cgi')

    def dir(self, *dirs):
        return apply(os.path.join,(self.project_dir,)+dirs)

    def keydir(self, *dirs):
        return apply(os.path.join,(self.config.config.key_dir,)+dirs)

    def logdir(self):
        return os.path.join(self.project_dir, "log_"+self.config.config.host)

    def create_keys(self):
        if not os.path.exists(self.keydir()):
            os.mkdir(self.keydir())
        _gen_key(self.keydir('upload'))
        _gen_key(self.keydir('code_sign'))

    def create_logdir(self):
        os.mkdir(self.logdir())
        os.chmod(self.logdir(), 02770)

    def keys_exist(self):
        keys = ['upload_private', 'upload_public',
                'code_sign_private', 'code_sign_public' ]
        for key in keys:
            if not os.path.exists(self.keydir(key)): return False
        return True

    # create new project.  Called only from make_project
    def install_project(self, scheduler_file = None):
        if os.path.exists(self.dir()):
            raise SystemExit('Project directory "%s" already exists; this would clobber it!'%self.dir())

        verbose_echo(1, "Creating directories");

        create_project_dirs(self.project_dir);

        if not self.web_only:
            if not self.keys_exist():
                verbose_echo(1, "Generating encryption keys");
                self.create_keys()

        # copy the user and administrative PHP files to the project dir,
        verbose_echo(1, "Copying files")

        # Create the project log directory
        self.create_logdir()

        install_boinc_files(self.dir(), True, not self.web_only)

        # copy sample web files to final names
        install(srcdir('html/user/sample_index.php'),
            self.dir('html/user/index.php'))
        install(srcdir('html/project.sample/project.inc'),
            self.dir('html/project/project.inc'))
        install(srcdir('html/project.sample/project_specific_prefs.inc'),
            self.dir('html/project/project_specific_prefs.inc'))
        install(srcdir('html/project.sample/cache_parameters.inc'),
            self.dir('html/project/cache_parameters.inc'))
        install(srcdir('tools/project.xml'), self.dir('project.xml'))
        if not self.production:
            install(srcdir('test/uc_result'), self.dir('templates/uc_result'))
            install(srcdir('test/uc_wu_nodelete'), self.dir('templates/uc_wu'))

        my_symlink(self.config.config.download_dir, self.dir('html', 'user', 'download'))
        my_symlink('../stats', self.dir('html/user/stats'))

        # Copy the sched server in the cgi directory with the cgi names given
        # source_dir/html/user/schedulers.txt
        #

        if scheduler_file:
            r = re.compile('<scheduler>([^<]+)</scheduler>', re.IGNORECASE)
            f = open(self.dir('html/user', scheduler_file))
            for line in f:
                # not sure if this is what the scheduler file is supposed to
                # mean
                match = r.search(line)
                if match:
                    cgi_name = match.group(1)
                    verbose_echo(2, "Copying " + cgi_name);
                    install(builddir('sched/cgi'), self.dir('cgi-bin', cgi_name,''))
            f.close()
        else:
            scheduler_file = 'schedulers.txt'
            f = open(self.dir('html/user', scheduler_file), 'w')
            print >>f, "<!-- <scheduler>" + self.scheduler_url.strip() + "</scheduler> -->"
            print >>f, "<link rel=\"boinc_scheduler\" href=\"" + self.scheduler_url.strip()+ "\">"
            f.close()

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
            force_symlink(self.dir('cgi-bin'), os.path.join(options.cgi_dir, self.short_name))
        if options.__dict__.get('html_dir'):
            force_symlink(self.dir('html/user'), os.path.join(options.html_dir, self.short_name))
            force_symlink(self.dir('html/ops'), os.path.join(options.html_dir, self.short_name+'_admin'))

    def http_password(self, user, password):
        'Adds http password protection to the html/ops directory'
        passwd_file = self.dir('html/ops', '.htpassword')
        f = open(self.dir('html/ops', '.htaccess'), 'w')
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
        cmdline = apply(build_command_line, [''], kwargs)
        if each_app:
            return map(lambda av: '-app %s %s'%(av.app.name,cmdline), self.app_versions)
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

def query_noyes(str):
    verbose_echo(0,'')
    return tools.query_noyes(str)

def query_yesno(str):
    verbose_echo(0,'')
    return tools.query_yesno(str)
