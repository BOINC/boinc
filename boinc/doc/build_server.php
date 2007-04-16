<? // -*- html -*-
   // $Id$
   require_once("docutil.php");
   page_head("Building the server");
?>

See the <a href=software.php>Software Prerequisites</a>.

<h2>Overview</h2>
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
Check:
<pre>
  make check
</pre>

<h2>Troubleshooting</h2>
<h3>MySQL</h3>
BOINC gets MySQL compiler and linker flags from a program
called <code>mysql_config</code> which comes with your MySQL distribution.
This sometimes references libraries that are not part of your base system
installation, such as <code>-lnsl</code> or <code>-lnss_files</code>.
You may need to install additional packages
(often you can use something called "mysql-dev" or "mysql-devel")
or fiddle with Makefiles.

<?
   page_tail();
?>
