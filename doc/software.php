<? // -*- html -*-
require_once("docutil.php");
page_head("Software prerequisites");
?>

BOINC depends on various software to build, test, and run.

<h2>Operating systems</h2>

The server components run on flavors of Unix.
We develop on Solaris 2.6-2.9, Red Hat 8,
and Debian Linux stable and unstable, so those currently work out-of-the-box.
Other Unix-like systems should work without too much configuration.

<h2>Other software</h2>

Required for <b>compiling</b>:
<ul>
  <li><b>GNU C++</b> 2.95 or 3.0-3.3
  <li>Other GNU development tools: gmake, gzip, etc.
</ul>

Required on the <b>database</b> server:
<ul>
  <li><b>MySQL server</b> 3.23+ or 4.0+
      (package <code>mysql-server</code>).

</ul>

Required on the <b>master/scheduler</b> server(s):
<ul>
  <li><b>Apache</b> or other webserver (package <code>apache2</code> or <code>apache</code>)
  <li><b>PHP</b> 4.0 (package <code>php4</code>)
  <li><b>MySQL client</b> (package <code>mysql-client</code>)
  <li><b>Python</b> 2.2+ (package <code>python2.3</code> or <code>python2.2</code>)
    <ul>
      <li><a href=http://sourceforge.net/projects/mysql-python><b>Python module MySQLdb</b></a>
        0.9.2 (0.9.1 currently won't work; see <a
              href=install_python_mysqldb.txt>installation instructions</a>)
        (package <code>python-mysqldb</code>)
      <li><a href=http://pyxml.sourceforge.net/><b>Python module xml</b></a>
          (part of most Python distributions; package <code>python-xml</code>)
    </ul>
</ul>

The <a href=test.php>test suite</a> simulates all servers and client on a
single machine, so to run <code>make check</code> you need most of the usual
server and client software:
<ul>
  <li><b>MySQL server</b> with permissions to create databases
  <li><b>MySQL client</b>
  <li><b>Python</b> with modules as above
  <li>Apache and PHP: can be used but not required
</ul>

Optional, required only if you change <code>*/Makefile.am</code>:
<ul>
  <li><b>automake</b> 1.7+
  <li><b>autoconf</b> 2.5+
</ul>


<small>
  On Debian Linux you can install all of the above software using
  <blockquote>
    <code>apt-get install g++ python python-mysqldb python-xml mysql-server mysql-client apache php4 automake autoconf</code>
  </blockquote>
</small>


<h2>Windows client software</h2>
Required for compiling:
<ul>
  <li><b>Microsoft Visual C++</b> 7.0
</ul>
Required for creating install packages:
<ul>
  <li><b>InstallShield</b> 5.5
</ul>


<h2>Macintosh client software</h2>
Required for compiling and creating install packages:
<ul>
  <li>Development Level PPC Macintosh running OS X 10.1 or later.
  <li>July 2002 Mac OS X Developer Tools.
  <li>Installer Vise Lite 3.6 SDK(?)
</ul>


<? page_tail(); ?>
