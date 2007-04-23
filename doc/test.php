<?php
require_once("docutil.php");
page_head("Test framework for BOINC developers (not intended for general use)");
echo "
See the section on testing in <a href=http://boinc.berkeley.edu/trac/wiki/SoftwarePrereqsUnix>Software Prerequisites</a>.
Note that a web server with PHP is required for
running a real server, but that requires a lot of path and permissions
configuration so we opt not to use them in testing.

<h2>Python testing framework</h2>

The <code>test/</code> directory contains a library of Python modules that
make end-to-end testing of BOINC easy.
See the <a href=python.php>Python framework</a>.

<h2>Quick start</h2>
Single test:
<blockquote>
  <code> cd boinc/test && ./test_uc.py </code>
</blockquote>

Full test suite:
<blockquote>
  <code> cd boinc/test && make check </code>
</blockquote>


After two or three minutes you should see 'Passed Test'.
During the test you should see various progress status indicators
(resize your window so that the status indicator line fits on one line).

<h2>Goals of the testing framework</h2>

The goal of the framework is to support automated testing of BOINC
and BOINC applications.
Each test is performed by a Python program that
initializes the system to a deterministic state, executes the system, detects
termination, and returns success or failure depending on the final state.

<p>

Many BOINC features involve multiple projects and/or multiple hosts.
It would be prohibitively complex to do automated testing on multiple
physical hosts.
Fortunately the BOINC architecture makes it possible for multiple projects and
(virtual) hosts to reside on a single physical host.
So the testing framework uses this approach.

<p>
The framework lets you test systems with any of the following attributes:
<ul>
<li> Multiple projects
<li> Multiple applications per project
<li> Multiple participants per project
<li> Multiple hosts, with arbitrary enrollment in projects
</ul>
The following system attributes are planned but not implemented yet:
<ul>
<li> Multiple data servers or scheduling servers per project.
<li> Simulated failures of servers and network connections.
<li> Simulations of various host conditions
(out of disk space, crash/reboot, etc.).
</ul>

<p>
TODO: Directory structure

<p>
TODO: Modules/ Classes

<p>
TODO: Example script

<h2>Test applications</h2>
The <code>apps</code> directory contains the following test applications:
<ul>
<li><code>upper_case</code>: converts a file to upper case.
<li><code>concat</code>: concatenates its input files.
<li><code>1sec</code>: uses 1 second of CPU time.
</ul>

<h2>Test cases</h2>
The <code>test/</code> directory contains various python scripts named with
prefix <code>test_</code>.
Each of these is a test case which runs an end-to-end test of BOINC
(creates directory structure for project(s) and
host(s), starts a web server, server daemons, host client(s), and checks
output is as expected).
<table border=1>
  <tr><td><code>test_uc.py</code></td>
    <td>The basic test using <code>uppercase</code></td></tr>
  <tr><td><code>test_concat.py</code></td>
    <td> tests command-line arguments and
      filenames</tr>
  <tr><td><code>test_uc_slow.py</code></td>
    <td> tests checkpoint/restart mechanism</tr>
  <tr><td><code>test_prefs.py</code></td>
    <td> tests some aspects of preferences.</tr>
  <tr><td><code>test_water.py</code></td>
    <td> tests some aspects of water marks.</tr>
  <tr><td><code>test_rsc.py</code></td>
    <td> tests that scheduling server only sends
      feasible work units.</tr>
  <tr><td><code>test_pers.py</code></td>
    <td> tests the persistent file transfers for
      download and upload.  It interrupts them in the middle and makes sure
      that the filesize never decreases along interrupted transfers.</tr>
  <tr><td><code>test_masterurl_failure.py</code></td>
    <td> tests the exponential backoff
      mechanism on the client in case of master URL failures.  This test is
      not automated.  It has to be run, and then client.out (in the host
      directory) must be looked at to examine whether everything is working
      correctly.</tr>
  <tr><td><code>test_sched_failure.py</code></td>
    <td>tests the exponential backoff mechanism
      on the client in case of scheduling server failures.
      This test is not automated.
      It has to be run, and then client.out (in the host
      directory) must be looked at to examine whether everything
      is working correctly.</tr>
</table>

<h2>Appendix: Optional Environment Variables</h2>
The following environment variables are optional and apply whatever web server
you use:
<p>

<code>BOINC_TEST_USER_NAME</code>
<blockquote>
  User name to use for setting up database name.
  Defaults to <code>\$USER</code>
</blockquote>

<code>BOINC_TEST_VERBOSE</code>
<blockquote>
  Verbosity level.
  <table border=1>
    <tr><td><code>0</code></td><td>print nothing</td></tr>
    <tr><td><code>1</code> [default]</td><td>print some.
    If output is a tty, overwrite lines.</td>
    <tr><td><code>2</code></td><td>print all</td>
  </table>
</blockquote>

<code>BOINC_TEST_DELETE</code>
<blockquote>
    Specifies whether or not to delete testbed after the test finishes.
    Only relevant when BOINC_TEST_AUTO_SETUP=0.
    Possible values (case doesn't matter):
    <table border=1>
      <tr><td><code>No</code></td></tr>
      <tr><td><code>If-Successful</code>   [default]</td></tr>
      <tr><td><code>Always</code></td></tr>
    </table>
</blockquote>
<code>BOINC_TEST_INSTALL_METHOD</code>
<blockquote>
    Specifies how to install html/php, CGI from source directories to testbed
    location.
    <table border=1>
      <tr><td><code>link</code>             </td><td>hardlink</td></tr>
      <tr><td><code>symlink</code> [default]</td><td>symbolic link</td></tr>
      <tr><td><code>copy</code>             </td><td>copy</td></tr>
    </table>
    Copying is useful because it preserves what version of the file was used
    in a particular test run and hardlinking is best because compiled cgi and
    scheduler programs are not disrupted by parallel builds.
</blockquote>

<h2>Appendix: Web Server</h2>

By default, the test script will use a custom web server that has no security
and minimal cgi/php capability.
You can also use Apache or some other web server with manually
initialized directories by setting these environment variables:

<blockquote>
<pre>
BOINC_TEST_AUTO_SETUP=0   [default=1]
BOINC_TEST_KEY_DIR
BOINC_TEST_PROJECTS_DIR
BOINC_TEST_CGI_URL
BOINC_TEST_HTML_URL
BOINC_TEST_CGI_DIR
BOINC_TEST_HTML_DIR
BOINC_TEST_HOSTS_DIR
</pre>
</blockquote>

<h3>Example setup</h3>
Bourne shell:
<pre>
    QHOME=/disks/philmor/a/users/quarl/proj
    TOP=\$QHOME/test-boinc
    URL=http://milhouse.ssl.berkeley.edu/quarl

    export BOINC_TEST_PROJECTS_DIR=\$TOP/projects
    export BOINC_TEST_USER_NAME=quarl
    export BOINC_TEST_SRC_DIR=\$QHOME/boinc
    export BOINC_TEST_CGI_DIR=\$TOP/boinc_cgi
    export BOINC_TEST_CGI_URL=\$URL/boinc_cgi
    export BOINC_TEST_HTML_DIR=\$TOP/boinc_html
    export BOINC_TEST_HTML_URL=\$URL/boinc_html
    export BOINC_TEST_SHMEM_KEY=0x1717f00f
    export BOINC_TEST_KEY_DIR=\$TOP/keys
    export BOINC_TEST_HOSTS_DIR=\$TOP/host
</pre>

Apache configuration:
<pre>
    &lt;Directory /disks/philmor/a/users/quarl/proj/test-boinc&gt;
            AllowOverride FileInfo AuthConfig Limit
            Options Indexes SymLinksIfOwnerMatch IncludesNoExec ExecCGI
        &lt;Limit GET POST OPTIONS PROPFIND&gt;
            Order allow,deny
            Allow from all
        &lt;/Limit&gt;
    &lt;/Directory&gt;

    ScriptAlias /quarl/boinc_cgi/ \"/disks/philmor/a/users/quarl/proj/test-boinc/boinc_cgi/\"
    Alias /quarl/ \"/disks/philmor/a/users/quarl/proj/test-boinc/\"
</pre>

";
page_tail();
?>
