<? // -*- html -*-
   // $Id$
   require_once("docutil.php");
   page_head("Building the server");
?>

<h1>Overview</h1>
Download:
<pre>
  wget http://boinc.berkeley.edu/source/boinc-VERSION.tar.gz
  tar xvzf boinc-VERSION.tar.gz
  cd boinc-VERSION
</pre>
Configure:
<pre>
  ./configure
</pre>
Make:
<pre>
  make
</pre>

<h1>Software</h1>
Compiling the server is supported directly on:
<ul><li>Linux
  <li>Solaris 2.7+
  <li>FreeBSD
</ul>
<ul><li>Other modern UNIX systems should be easy to port</ul>

Compiler:
<ul><li>g++ (2.95 or 3.0-3.3)</ul>

Libraries:
<ul><li>MySQL</ul>

<h1>Troubleshooting</h1>
<h2>MySQL</h2> BOINC gets MySQL compiler and linker flags from a program
called <code>mysql_config</code> which comes with your MySQL distribution.
This sometimes references libraries that are not part of your base system
installation, such as <code>-lnsl</code> or <code>-lnss_files</code>.  You may
need to install additional packages (often you can use something called
"mysql-dev" or "mysql-devel") or fiddle with Makefiles.

<?
   page_tail();
?>
