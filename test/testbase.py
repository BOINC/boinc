## $Id$

# testbase.py
#
# A set of classes for testing BOINC.  These classes let you create multiple
# projects and multiple hosts (all running on a single machine), add
# applications and work units, run the system, and verify that the results are
# correct.
#
# See doc/test.html for details

# the module MySQLdb can be installed on debian with "apt-get install python2.2-mysqldb"

# TODO: make things work if build_dir != src_dir

import sys
sys.path.append('../py')
try:
    from version import *
except ImportError:
    raise SystemExit("""testbase.py: Couldn't import version.py

This file is built from py/version.py.in by configure.

Perhaps you did not run configure, or you configured in a different directory,
or you are running from the wrong directory.""")

from boinc import *
import atexit, traceback, signal

# raise SystemExit('hi')
# raise Exception('Hi')

options.have_init_t = False

def test_init():
    if options.have_init_t: return
    options.have_init_t = True

    if not os.path.exists('test_uc.py'):
        os.chdir(os.path.join(TOP_SRC_DIR,'test'))
    if not os.path.exists('test_uc.py'):
        raise SystemExit('Could not find boinc_db.py anywhere')

    options.auto_setup     = int(get_env_var("BOINC_TEST_AUTO_SETUP",0)) ####
    options.user_name      = get_env_var("BOINC_TEST_USER_NAME", '') or get_env_var("USER")
    options.delete_testbed = get_env_var("BOINC_TEST_DELETE", 'if-successful').lower()
    options.install_method = get_env_var("BOINC_TEST_INSTALL_METHOD", 'hardlink').lower()
    options.echo_verbose   = int(get_env_var("BOINC_TEST_VERBOSE", '1'))
    options.proxy_port     = 16000 + (os.getpid() % 1000)
    options.drop_db_first  = True
    
    if options.auto_setup:
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
        options.miniserv_basedir = os.path.join(os.getcwd(), options.auto_setup_basedir)
        options.miniserv_port    = 15000 + (os.getpid() % 1000)
        options.miniserv_baseurl = 'http://localhost:%d/' % options.miniserv_port
        miniserver = MiniServer(options.miniserv_port, options.miniserv_basedir)
        miniserver.run()
        options.projects_dir = os.path.join(options.miniserv_basedir, 'projects')
        options.cgi_dir      = os.path.join(options.miniserv_basedir, 'cgi-bin')
        options.html_dir     = os.path.join(options.miniserv_basedir, 'html')
        options.hosts_dir    = os.path.join(options.miniserv_basedir, 'hosts')
        options.cgi_url      = os.path.join(options.miniserv_baseurl, 'cgi-bin')
        options.html_url     = os.path.join(options.miniserv_baseurl, 'html')
        options.port         = options.miniserv_port
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
            if value == None:
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

class ProjectList(list):
    def run(self):   map(lambda i: i.run(), self)
    def check(self): map(lambda i: i.check(), self)
    def stop(self):  map(lambda i: i.maybe_stop(), self)
    def get_progress(self):
        s = "Running core client - results [done/total]:"
        for db in self.dbs:
            s += " [%d/%d]" % (num_results_done(db), num_results(db))
        return s
    def start_progress_meter(self):
        self.dbs = map(lambda i: i.db_open(), self)
        self.rm = ResultMeter(self.get_progress)
    def stop_progress_meter(self):
        self.rm.stop()

all_projects = ProjectList()

class TestProject(Project):
    def __init__(self, works, users=None, hosts=None,
                 add_to_list=True, **kwargs):
        test_init()
        if add_to_list:
            all_projects.append(self)

        kwargs['short_name'] = kwargs.get('short_name') or 'test_'+kwargs['appname']
        kwargs['long_name'] = kwargs.get('long_name') or 'Project ' + kwargs['short_name'].replace('_',' ').capitalize()
        self.works = works
        self.users = users or [User()]
        self.hosts = hosts or [Host()]
        # convenience vars:
        self.work  = self.works[0]
        self.user  = self.users[0]
        self.host  = self.hosts[0]

        apply(Project.__init__, [self], kwargs)
        self.started = False

    def init_install(self):
        if not options.auto_setup:
            verbose_echo(1, "Deleting previous test runs")
            rmtree(self.dir())
        # self.drop_db_if_exists()

    def query_create_keys(self):
        '''Overrides Project::query_create_keys() to always return true'''
        return True

    def install_project_users(self):
        db = self.db_open()
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

    def install_works(self):
        for work in self.works:
            work.install(self)

    def install_hosts(self):
        for host in self.hosts:
            for user in self.users:
                host.add_user(user, self)
            host.install()

    def install(self):
        self.init_install()
        self.install_project()
        self.install_project_users()
        self.install_works()
        self.install_hosts()

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
        db = self.db_open()
        rows = db_query(db,"select * from result")
        for row in rows:
            dict_match(matchresult, row)
        db.close()
        if len(rows) != expected_count:
            error("expected %d results, but found %d" % (expected_count, len(rows)))

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

class HostList(list):
    def run(self):   map(lambda i: i.run(), self)

all_hosts = HostList()

class Host:
    def __init__(self, add_to_list=True):
        if add_to_list:
            all_hosts.append(self)
        self.name = 'Commodore64'
        self.users = []
        self.projects = []
        self.global_prefs = None
        self.log_flags = 'log_flags.xml'
        self.host_dir = os.path.join(options.hosts_dir, self.name)
        self.defargs = "-exit_when_idle -skip_cpu_benchmarks -debug_fake_exponential_backoff"
        # self.defargs = "-exit_when_idle -skip_cpu_benchmarks -sched_retry_delay_min 1"

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
        verbose_shell_call("cd %s && %s %s %s > client.out 2> client.err" % (
            self.dir(), builddir('client', options.client_bin_filename),
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
    def __init__(self, redundancy=1):
        self.input_files = []
        self.rsc_iops = 1.8e12
        self.rsc_fpops = 1e13
        self.rsc_memory = 1e7
        self.rsc_disk = 1e7
        self.delay_bound = 1000
        self.redundancy = redundancy
        self.app = None

    def install(self, project):
        verbose_echo(1, "Installing work <%s> in project '%s'" %(
            self.wu_template, project.short_name))
        if not self.app:
            self.app = project.app_versions[0].app
        for input_file in unique(self.input_files):
            install(input_file, os.path.join(project.download_dir,''))

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

class ResultMeter:
    def __init__(self, func, args=[], delay=.1):
        '''Forks to print a progress meter'''
        self.pid = os.fork()
        if self.pid:
            atexit.register(self.stop)
            return
        while True:
            verbose_echo(1, apply(func, args))
            time.sleep(delay)
    def stop(self):
        if self.pid:
            os.kill(self.pid, 9)
            self.pid = 0

def run_check_all():
    '''Run all projects, run all hosts, check all projects, stop all projects.'''
    atexit.register(all_projects.stop)
    all_projects.run()
    all_projects.start_progress_meter()
    if os.environ.get('TEST_STOP_BEFORE_HOST_RUN'):
        raise SystemExit, 'Stopped due to $TEST_STOP_BEFORE_HOST_RUN'
    all_hosts.run()
    all_projects.stop_progress_meter()
    all_projects.check()
    all_projects.stop()

def delete_test():
    '''Delete all test data'''
    if options.auto_setup:
        verbose_echo(1, "Deleting testbed %s."%options.auto_setup_basedir)
        shutil.rmtree(options.auto_setup_basedir)

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
            fatal_error("testproxy failed")
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


class MiniServer:
    def __init__(self, port, doc_root, miniserv_root=None):
        self.port = port
        self.doc_root = doc_root
        self.miniserv_root = miniserv_root or os.path.join(doc_root,'miniserv')
        if not os.path.isdir(self.miniserv_root):
            os.mkdir(self.miniserv_root)
        self.config_file = os.path.join(self.miniserv_root, 'miniserv.conf')
        self.log_file = os.path.join(self.miniserv_root, 'miniserv.log')
        self.pid_file = os.path.join(self.miniserv_root, 'miniserv.pid')
        print >>open(self.config_file,'w'), '''
root=%(doc_root)s
mimetypes=/etc/mime.types
port=%(port)d
addtype_cgi=internal/cgi
addtype_php=internal/cgi
index_docs=index.html index.htm index.cgi index.php
logfile=%(log_file)s
pidfile=%(pid_file)s
logtime=168
ssl=0
#logout=/etc/webmin/logout-flag
#libwrap=1
#alwaysresolve=1
#allow=127.0.0.1
blockhost_time=300
no_pam=0
logouttime=5
passdelay=1
blockhost_failures=3
log=1
logclear=
loghost=1
''' %self.__dict__

    def run(self):
        verbose_echo(0,"Running miniserv on localhost:%d"%self.port)
        if os.spawnl(os.P_WAIT, srcdir('test/miniserv.pl'), 'miniserv', self.config_file):
            raise SystemExit("Couldn't spawn miniserv")
        atexit.register(self.stop)

    def stop(self):
        verbose_echo(1,"Killing miniserv")
        try:
            pid = int(open(self.pid_file).readline())
            os.kill(pid, signal.SIGINT)
        except Exception, e:
            print >>sys.stderr, "Couldn't stop miniserv:", e

def test_msg(msg):
    print
    print "-- Testing", msg, '-'*(66-len(msg))
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
        sys.exit(int(errors.count))
    else:
        verbose_echo(1, "Passed test!")
        if options.echo_overwrite:
            print
        if options.delete_testbed == 'if-successful' or options.delete_testbed == 'always':
            delete_test()
        if options.echo_overwrite:
            print
        sys.exit(0)

atexit.register(test_done)
