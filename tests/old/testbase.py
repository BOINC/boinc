## $Id$

# testbase.py
#
# A set of classes for testing BOINC.
# These classes let you create multiple projects and multiple hosts
# (all running on a single machine), add applications and work units,
# run the system, and verify that the results are correct.
#
# See doc/test.php for details

# TODO: make sure things work if build_dir != src_dir

import boinc_path_config
from Boinc import version
from Boinc.setup_project import *
import atexit, traceback, signal
import cgiserver

options.have_init_t = False
options.echo_overwrite = False
options.client_bin_filename = version.CLIENT_BIN_FILENAME

def test_init():
    if options.have_init_t: return
    options.have_init_t = True

    if not os.path.exists('testbase.py'):
        # automake sets the srcdir env. variable if srcdir != builddir
        os.chdir(os.path.join(os.getenv('srcdir'), 'tests', 'old'))
    if not os.path.exists('testbase.py'):
        raise SystemExit('Could not find testbase.py anywhere')

    #options.program_path = os.path.realpath(os.path.dirname(sys.argv[0]))
    options.program_path = os.getcwd()

    options.auto_setup     = int(get_env_var("BOINC_TEST_AUTO_SETUP",1))#######
    options.user_name      = get_env_var("BOINC_TEST_USER_NAME", '') or get_env_var("USER")
    options.db_user        = options.user_name
    options.db_passwd      = ''
    options.db_host        = ''
    options.delete_testbed = get_env_var("BOINC_TEST_DELETE", 'if-successful').lower()
# options.install_method = get_env_var("BOINC_TEST_INSTALL_METHOD", 'symlink').lower()
    options.install_method = 'copy'
    options.echo_verbose   = int(get_env_var("BOINC_TEST_VERBOSE", '1'))
    options.proxy_port     = 16000 + (os.getpid() % 1000)
    options.drop_db_first  = True

    if options.auto_setup:
        cgiserver.setup_php(program_path = options.program_path)

        options.auto_setup_basedir = 'run-%d'%os.getpid()
        verbose_echo(0, "Creating testbed in %s"%options.auto_setup_basedir)
        os.mkdir(options.auto_setup_basedir)
        try:
            os.unlink('run')
        except OSError:
            pass
        try:
            os.symlink(options.auto_setup_basedir, 'run')
        except OSError:
            pass
        options.cgiserver_basedir = os.path.join(os.getcwd(), options.auto_setup_basedir)
        options.cgiserver_port    = 15000 + (os.getpid() % 1000)
        options.cgiserver_baseurl = 'http://localhost:%d/' % options.cgiserver_port
        CgiServer = AsynchCGIServer()
        CgiServer.serve(base_dir=options.auto_setup_basedir, port=options.cgiserver_port)
        options.projects_dir = os.path.join(options.cgiserver_basedir, 'projects')
        options.cgi_dir      = os.path.join(options.cgiserver_basedir, 'cgi-bin')
        options.html_dir     = os.path.join(options.cgiserver_basedir, 'html')
        options.hosts_dir    = os.path.join(options.cgiserver_basedir, 'hosts')
        options.cgi_url      = os.path.join(options.cgiserver_baseurl, 'cgi-bin')
        options.html_url     = os.path.join(options.cgiserver_baseurl, 'html')
        options.port         = options.cgiserver_port
        map(os.mkdir, [options.projects_dir, options.cgi_dir, options.html_dir, options.hosts_dir])
    else:
        options.key_dir      = get_env_var("BOINC_TEST_KEY_DIR")
        options.projects_dir = get_env_var("BOINC_TEST_PROJECTS_DIR")
        options.cgi_dir      = get_env_var("BOINC_TEST_CGI_DIR")
        options.html_dir     = get_env_var("BOINC_TEST_HTML_DIR")
        options.hosts_dir    = get_env_var("BOINC_TEST_HOSTS_DIR")
        options.cgi_url      = get_env_var("BOINC_TEST_CGI_URL")
        options.html_url     = get_env_var("BOINC_TEST_HTML_URL")
        m = re.compile('http://[^/]+:(\d+)/').match(options.html_url)
        options.port = m and m.group(1) or 80

    init()

def proxerize(url, t=True):
    if t:
        r = re.compile('http://[^/]*/')
        return r.sub('http://localhost:%d/'%options.proxy_port, url)
    else:
        return url

def use_cgi_proxy():
    test_init()
    options.cgi_url = proxerize(options.cgi_url)
def use_html_proxy():
    test_init()
    options.html_url = proxerize(options.html_url)

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

class STARTS_WITH(str):
    pass

class MATCH_REGEXPS(list):
    def match(self, text):
        '''Returns True iff each regexp in self is in text'''
        for r in self:
            R = re.compile(r)
            if not R.search(text):
                return False
        return True
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
        elif isinstance(expected,MATCH_REGEXPS):
            match = expected.match(found)
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

class ExpectedResult:
    def __init__(self):
        self.server_state = RESULT_SERVER_STATE_OVER
        self.client_state = RESULT_FILES_UPLOADED
        self.outcome      = RESULT_OUTCOME_SUCCESS

class ExpectedResultComputeError:
    def __init__(self):
        self.server_state = RESULT_SERVER_STATE_OVER
        self.client_state = RESULT_COMPUTE_DONE
        self.outcome      = RESULT_OUTCOME_CLIENT_ERROR

# TODO: figure out a better way to change settings for the progress meter than
# kill and refork
class ProjectList(list):
    def __init__(self):
        list.__init__(self)
        self.state = '<error>'
        self.pm_started = False
    def install(self):
        self.state = '<error>'
        for i in self:
            i.install()
    def run(self):
        self.set_progress_state('INIT ')
        for i in self:
            i.run()
    def run_init_wait(self):
        for i in self:
            i.run_init_wait()
        self.set_progress_state('RUNNING ')
    def run_finish_wait(self):
        self.set_progress_state('FINISH ')
        for i in self:
            i.run_finish_wait()
        self.state='DONE '
    def check(self):
        for i in self:
            i.check()
    def stop(self):
        for i in self:
            i.maybe_stop()
    def get_progress(self):
        progress = []
        for project in self:
            progress.append(project.short_name + ": " + project.progress_meter_status())
        return self.state + " " + ' | '.join(progress)
    def start_progress_meter(self):
        for project in self:
            project.progress_meter_ctor()
        self.rm = ResultMeter(self.get_progress)
        self.pm_started = True
    def stop_progress_meter(self):
        if self.pm_started:
            self.rm.stop()
            for project in self:
                project.progress_meter_dtor()
            self.pm_started = False
    def restart_progress_meter(self):
        if self.pm_started:
            self.stop_progress_meter()
            self.start_progress_meter()
    def set_progress_state(self, state):
        self.state = state
        self.restart_progress_meter()
all_projects = ProjectList()

DEFAULT_NUM_WU = 10
DEFAULT_REDUNDANCY = 5

def get_redundancy_args(num_wu = None, redundancy = None):
    num_wu = num_wu or sys.argv[1:] and get_int(sys.argv[1]) or DEFAULT_NUM_WU
    redundancy = redundancy or sys.argv[2:] and get_int(sys.argv[2]) or DEFAULT_REDUNDANCY
    return (num_wu, redundancy)

class TestProject(Project):
    def __init__(self, works, expected_result, appname=None,
                 num_wu=None, redundancy=None,
                 users=None, hosts=None,
                 add_to_list=True,
                 apps=None, app_versions=None,
                 resource_share=None,
                 **kwargs):
        test_init()
        if add_to_list:
            all_projects.append(self)

        kwargs['short_name'] = kwargs.get('short_name') or 'test_'+appname
        kwargs['long_name'] = kwargs.get('long_name') or 'Project ' + kwargs['short_name'].replace('_',' ').capitalize()
        super(TestProject, self).__init__(**kwargs)

        (num_wu, redundancy) = get_redundancy_args(num_wu, redundancy)
        self.resource_share = resource_share or 1
        self.num_wu = num_wu
        self.redundancy = redundancy
        self.expected_result = expected_result
        self.works = works
        self.users = users or [User()]
        self.hosts = hosts or [Host()]

        self.platforms     = [Platform()]
        self.app_versions  = app_versions or [
            AppVersion(App(appname), self.platforms[0], appname)]
        self.apps          = apps or unique(map(lambda av: av.app, self.app_versions))
        # convenience vars:
        self.app_version   = self.app_versions[0]
        self.app           = self.apps[0]

        # convenience vars:
        self.work  = self.works[0]
        self.user  = self.users[0]
        self.host  = self.hosts[0]
        self.started = False

    def init_install(self):
        if not options.auto_setup:
            verbose_echo(1, "Deleting previous test runs")
            rmtree(self.dir())
        # self.drop_db_if_exists()

    def query_create_keys(self):
        '''Overrides Project::query_create_keys() to always return true'''
        return True

    def install_works(self):
        for work in self.works:
            work.install(self)

    def install_hosts(self):
        for host in self.hosts:
            for user in self.users:
                host.add_user(user, self)
            host.install()

    def install_platforms_versions(self):
        def commit(list):
            for item in list: item.commit()

        self.platforms = unique(map(lambda a: a.platform, self.app_versions))
        verbose_echo(1, "Setting up database: adding %d platform(s)" % len(self.platforms))
        commit(self.platforms)

        verbose_echo(1, "Setting up database: adding %d apps(s)" % len(self.apps))
        commit(self.apps)

        verbose_echo(1, "Setting up database: adding %d app version(s)" % len(self.app_versions))
        commit(self.app_versions)

        verbose_echo(1, "Setting up database: adding %d user(s)" % len(self.users))
        commit(self.users)

    def install(self):
        self.init_install()
        self.install_project()
        self.install_platforms_versions()
        self.install_works()
        self.install_hosts()

    def run(self):
        self.sched_install('make_work', max_wus = self.num_wu)
        self.sched_install('sample_dummy_assimilator')
        self.sched_install('file_deleter')
        self.sched_install('sample_bitwise_validator')
        self.sched_install('feeder')
        self.sched_install('transitioner')
        self.start_servers()

    def run_init_wait(self):
        time.sleep(8)
    def run_finish_wait(self):
        '''Sleep until all workunits are assimilated and no workunits need to transition.

        If more than X seconds have passed than just assume something is broken and return.'''

        timeout = time.time() + 3*60
        while (num_wus_assimilated() < self.num_wu) or num_wus_to_transition():
            time.sleep(.5)
            if time.time() > timeout:
                error("run_finish_wait(): timed out waiting for workunits to assimilate/transition")
                break

    def check(self):
        # verbose_sleep("Sleeping to allow server daemons to finish", 5)
        # TODO:     self.check_outputs(output_expected,  ZZZ, YYY)
        self.check_results(self.expected_result, self.num_wu*self.redundancy)
        self.sched_run('file_deleter')
        # self.check_deleted("download/input")
        # TODO: use generator/iterator whatever to check all files deleted
        # self.check_deleted("upload/uc_wu_%d_0", count=self.num_wu)

    def progress_meter_ctor(self):
        pass
    def progress_meter_status(self):
        return "WUs: [A:%d T:%d]  Results: [U:%d,IP:%d,O:%d T:%d]" % (
            num_wus_assimilated(),
            #num_wus(),
            self.num_wu,
            num_results_unsent(),
            num_results_in_progress(),
            num_results_over(),
            num_results())
    def progress_meter_dtor(self):
        pass

    def _disable(self, *path):
        '''Temporarily disable a file to test exponential backoff'''
        path = self.dir(*path)
        os.rename(path, path+'.disabled')
    def _reenable(self, *path):
        path = self.dir(*path)
        os.rename(path+'.disabled', path)

    def disable_masterindex(self):
        self._disable('html_user/index.php')
    def reenable_masterindex(self):
        self._reenable('html_user/index.php')
    def disable_scheduler(self, num = ''):
        self._disable('cgi-bin/cgi'+str(num))
    def reenable_scheduler(self, num = ''):
        self._reenable('cgi-bin/cgi'+str(num))
    def disable_downloaddir(self, num = ''):
        self._disable('download'+str(num))
    def reenable_downloaddir(self, num = ''):
        self._reenable('download'+str(num))
    def disable_file_upload_handler(self, num = ''):
        self._disable('cgi-bin/file_upload_handler'+str(num))
    def reenable_file_upload_handler(self, num = ''):
        self._reenable('cgi-bin/file_upload_handler'+str(num))

    def check_results(self, matchresult, expected_count=None):
        '''MATCHRESULT should be a dictionary of columns to check, such as:

        server_state
        stderr_out
        exit_status
        '''
        expected_count = expected_count or self.redundancy
        results = database.Results.find()
        for result in results:
            dict_match(matchresult, result.__dict__)
        if len(results) != expected_count:
            error("expected %d results, but found %d" % (expected_count, len(results)))

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

class Platform(database.Platform):
    def __init__(self, name=None, user_friendly_name=None):
        database.Platform.__init__(self)
        self.name = name or version.PLATFORM
        self.user_friendly_name = user_friendly_name or name

class User(database.User):
    def __init__(self):
        database.User.__init__(self,id=None)
        self.name = 'John'
        self.email_addr = 'john@boinc.org'
        self.authenticator = "3f7b90793a0175ad0bda68684e8bd136"

class App(database.App):
    def __init__(self, name):
        database.App.__init__(self,id=None)
        self.name = name
        self.min_version = 1

class AppVersion(database.AppVersion):
    def __init__(self, app, platform, exec_file):
        database.AppVersion.__init__(self,id=None)
        self.app = app
        self.version_num = version.BOINC_MAJOR_VERSION * 100
        self.platform = platform
        self.min_core_version = 1
        self.max_core_version = 999
        self._exec_file=exec_file
    def commit(self):
        self.xml_doc = tools.process_app_version(
            self.app, self.version_num,
            [os.path.join(boinc_path_config.TOP_BUILD_DIR,'apps',self._exec_file)],
            quiet=True)
        database.AppVersion.commit(self)

class HostList(list):
    def run(self, asynch=False): map(lambda i: i.run(asynch=asynch), self)
    def stop(self):              map(lambda i: i.stop(), self)

all_hosts = HostList()

class Host:
    def __init__(self, add_to_list=True):
        if add_to_list:
            all_hosts.append(self)
        self.name = 'Commodore64'
        self.users = []
        self.projects = []
        self.global_prefs = None
        self.cc_config = 'cc_config.xml'
        self.host_dir = os.path.join(options.hosts_dir, self.name)
        self.defargs = "-exit_when_idle -skip_cpu_benchmarks -return_results_immediately"
        # self.defargs = "-exit_when_idle -skip_cpu_benchmarks -sched_retry_delay_bin 1"

    def add_user(self, user, project):
        self.users.append(user)
        self.projects.append(project)

    def dir(self, *dirs):
        return os.path.join(self.host_dir, *dirs)

    def install(self):
        rmtree(self.dir())
        os.mkdir(self.dir())

        verbose_echo(1, "Setting up host '%s': creating account files" % self.name)
        for (user,project) in map(None,self.users,self.projects):
            filename = self.dir(account_file_name(project.config.config.master_url))
            verbose_echo(2, "Setting up host '%s': writing %s" % (self.name, filename))

            with open(filename, "w") as f:
                f.write("<account>\n")
                f.write("{}\n".format(map_xml(project.config.config, ['master_url'])))
                f.write("{}\n".format(map_xml(user, ['authenticator'])))
                if user.project_prefs:
                    f.write("{}\n".format(user.project_prefs))
                f.write("</account>\n")

        # copy log flags and global prefs, if any
        if self.cc_config:
            shutil.copy(self.cc_config, self.dir('cc_config.xml'))
        # if self.global_prefs:
        #     shell_call("cp %s %s" % (self.global_prefs, self.dir('global_prefs.xml')))
        #     # shutil.copy(self.global_prefs, self.dir('global_prefs.xml'))

    def run(self, args='', asynch=False):
        if asynch:
            verbose_echo(1, "Running core client asynchronously")
            pid = os.fork()
            if pid:
                self.pid = pid
                return
        else:
            verbose_echo(1, "Running core client")
        verbose_shell_call("cd %s && %s %s %s > client.out 2> client.err" % (
            self.dir(), builddir('client', options.client_bin_filename),
            self.defargs, args))
        if asynch: os._exit(0)
    def stop(self):
        if self.pid:
            verbose_echo(1, "Stopping core client")
            try:
                os.kill(self.pid, 2)
            except OSError:
                verbose_echo(0, "Couldn't kill pid %d" % self.pid)
            self.pid = 0

    def read_cpu_time_file(self, filename):
        try:
            return float(open(self.dir(filename)).readline())
        except:
            return 0

    def check_file_present(self, project, filename):
        check_exists(self.dir('projects',
                              _url_to_filename(project.master_url),
                              filename))

        # TODO: do this in Python
class Work:
    def __init__(self, redundancy, **kwargs):
        self.input_files = []
        self.rsc_fpops_est = 1e10
        self.rsc_fpops_bound = 4e10
        self.rsc_memory_bound = 1e7
        self.rsc_disk_bound = 1e7
        self.delay_bound = 86400
        if not isinstance(redundancy, int):
            raise TypeError
        self.min_quorum = redundancy
        self.target_nresults = redundancy
        self.max_error_results = redundancy * 2
        self.max_total_results = redundancy * 4
        self.max_success_results = redundancy * 2
        self.app = None
        self.__dict__.update(kwargs)

    def install(self, project):
        verbose_echo(1, "Installing work <%s> in project '%s'" %(
            self.wu_template, project.short_name))
        if not self.app:
            self.app = project.app_versions[0].app
        for input_file in unique(self.input_files):
            install(os.path.realpath(input_file),
                    os.path.join(project.config.config.download_dir,
                                 os.path.basename(input_file)))

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

        shutil.copy(self.result_template, project.dir())
        cmd = build_command_line("create_work",
                                 config_dir          = project.dir(),
                                 appname             = self.app.name,
                                 rsc_fpops_est       = self.rsc_fpops_est,
                                 rsc_fpops_bound     = self.rsc_fpops_bound,
                                 rsc_disk_bound      = self.rsc_disk_bound,
                                 rsc_memory_bound    = self.rsc_memory_bound,
                                 wu_template         = self.wu_template,
                                 result_template     = self.result_template,
                                 min_quorum          = self.min_quorum,
                                 target_nresults     = self.target_nresults,
                                 max_error_results   = self.max_error_results,
                                 max_total_results   = self.max_total_results,
                                 max_success_results = self.max_success_results,
                                 wu_name             = self.wu_template,
                                 delay_bound         = self.delay_bound)

        for input_file in self.input_files:
            cmd += ' ' + input_file

        run_tool(cmd)

RESULT_METER_DELAY = 0.5
# Note: if test script errors with:
#   _mysql_exceptions.OperationalError: 2013, 'Lost connection to MySQL server
#   during query'
# Then your mysql server can't handle the load -- increase the RESULT_METER_DELAY

class ResultMeter:
    def __init__(self, func, args=[], delay=RESULT_METER_DELAY):
        '''Forks to print a progress meter'''
        self.pid = os.fork()
        if self.pid:
            atexit.register(self.stop)
            return
        # re-open database
        from Boinc import db_base, database
        db_base.dbconnection = None
        database.connect()
        prev_s = None
        while True:
            s = func(*args)
            if s != prev_s:
                verbose_echo(1, s)
                prev_s = s
            time.sleep(delay)
    def stop(self):
        if self.pid:
            os.kill(self.pid, 9)
            self.pid = 0

def run_check_all():
    '''Run all projects, run all hosts, check all projects, stop all projects.'''
    atexit.register(all_projects.stop)
    all_projects.install()
    all_projects.run()
    all_projects.start_progress_meter()
    all_projects.run_init_wait()
    if os.environ.get('TEST_STOP_BEFORE_HOST_RUN'):
        verbose_echo(1, 'stopped')
        # wait instead of killing backend procs.
        # (Is there a better way to do this?)
        while (1):
            time.sleep(1)
        raise SystemExit('Stopped due to $TEST_STOP_BEFORE_HOST_RUN')
    # all_hosts.run(asynch=True)
    all_hosts.run()
    all_projects.run_finish_wait()
    all_projects.stop_progress_meter()
    print()
    # all_hosts.stop()
    time.sleep(12)
    all_projects.stop()
    all_projects.check()

def delete_test():
    '''Delete all test data'''
    if options.auto_setup and hasattr(options,'auto_setup_basedir'):
        verbose_echo(1, "Deleting testbed %s."%options.auto_setup_basedir)
        try:
            shutil.rmtree(options.auto_setup_basedir)
        except Exception as e:
            verbose_echo(0, "Couldn't delete testbed %s: %s"%(options.auto_setup_basedir,e))

class Proxy:
    def __init__(self, code, cgi=0, html=0, start=1):
        self.pid = 0
        self.code = code
        if cgi:   use_cgi_proxy()
        if html:  use_html_proxy()
        if start: self.start()
    def start(self):
        self.pid = os.fork()
        if not self.pid:
            verbose_shell_call(
                "exec ./testproxy %d localhost:%d '%s' 2>testproxy.log" % (
                options.proxy_port, options.port, self.code),
                doexec=True)
        verbose_sleep("Starting proxy server", 1)
        # check if child process died
        (pid,status) = os.waitpid(self.pid, os.WNOHANG)
        if pid:
            fatal_error("testproxy failed; see testproxy.log for details")
            self.pid = 0
        else:
            atexit.register(self.stop)
    def stop(self):
        if self.pid:
            verbose_echo(1, "Stopping proxy server")
            try:
                os.kill(self.pid, 2)
            except OSError:
                verbose_echo(0, "Couldn't kill pid %d" % self.pid)
            self.pid = 0



class AsynchCGIServer:
    def __init__(self):
        self.pid = None
    def serve(self, port, base_dir):
        verbose_echo(0,"Running CGI server on localhost:%d"%port)
        self.pid = os.fork()
        if self.pid:
            atexit.register(self.kill)
            return
        ## child
        os.chdir(base_dir)
        sys.stderr = open('cgiserver.log', 'w', 0)
        cgiserver.serve(port=port)
        os._exit(1)
    def kill(self):
        if self.pid:
            verbose_echo(1,"Killing cgiserver")
            try:
                os.kill(self.pid, 9)
            except Exception:
                verbose_echo(0, "Couldn't kill cgiserver pid %d" %self.pid)
            self.pid = None

def test_msg(msg):
    print("\n-- Testing", msg, '-'*(66-len(msg)))
    test_init()

def test_done():
    test_init()
    if sys.__dict__.get('last_traceback'):
        if sys.last_type == KeyboardInterrupt:
            errors.count += 0.1
            sys.stderr.write("\nTest canceled by user\n")
        else:
            errors.count += 1
            sys.stderr.write("\nException thrown - bug in test scripts?\n")
    if errors.count:
        verbose_echo(0, "ERRORS.COUNT: %d" % errors.count)
        if options.delete_testbed == 'always':
            delete_test()
        return int(errors.count)
    else:
        verbose_echo(1, "Passed test!")
        if options.echo_overwrite:
            print()
        if options.delete_testbed == 'if-successful' or options.delete_testbed == 'always':
            delete_test()
        if options.echo_overwrite:
            print()
        return 0

# Note: this bypasses other cleanup functions - if something goes wrong during
# exit, this may be the culprit.
def osexit_test_done():
    os._exit(test_done())

atexit.register(osexit_test_done)
