<? // -*- html -*-
require_once("docutil.php");
page_head("BOINC: Software prerequisites and corequisites");
?>

BOINC depends on various software to build, test, and run.

<h1>Unix</h1>

Required for <b>compiling</b>:
<ul>
  <li><b>GNU C++</b> 2.95 or 3.0-3.3; other C++ compilers can be ported
  <li>Other standard development tools assumed: make, gzip, etc.
</ul>

Required to run automated tests, create a project and other various tools:
<ul>
  <li><b>Python</b> 2.2+
    <ul>
      <li>Python module <a
          href=http://sourceforge.net/projects/mysql-python><b>MySQLdb</b></a>
          0.9.2 (0.9.1 currently won't work; see <a href=install_python_mysqldb.txt>installation instructions</a>)
      <li>Python module <a href=http://pyxml.sourceforge.net/><b>xml</b></a> (part of most distributions)
    </ul>
</ul>

Required on the <b>database</b> server:
<ul>
  <li><b>MySQL server</b> 3.25+ or 4.0+: other SQL server can be ported.
</ul>

Required on the <b>master/scheduler</b> server(s):
<ul>
  <li><b>Apache</b> or other webserver
  <li><b>PHP</b> 4.0
  <li><b>MySQL client</b>
</ul>

Optional, required only if you change <code>*/Makefile.am</code>:
<ul>
  <li><b>automake</b> 1.7+
  <li><b>autoconf</b> 2.5+
</ul>


<small>
  On Debian Linux you can install all of the above software using
  <blockquote>
    <code>apt-get install g++ python python-mysqldb python-xml mysql-server mysql-client apache php automake autoconf</code>
  </blockquote>
</small>


<h1>Windows client</h1>
Required for compiling:
<ul>
  <li><b>Microsoft Visual C</b> 6.0
</ul>
Required for creating install packages:
<ul>
  <li><b>InstallShield</b>
</ul>


<h1>Macintosh client</h1>
Required for compiling:
<ul>
  <li><b>?</b>
</ul>


<? page_tail(); ?>
