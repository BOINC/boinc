<? // -*- html -*-
require_once("docutil.php");
page_head("BOINC: Build system");
?>

See the <a href=software.php>Software Prerequisites</a>.

<h1>Details of BOINC's new automake/autoconf build system</h1>
<p>
  The BOINC build system uses <a
  href=http://www.gnu.org/software/autoconf/>autoconf</a> 2.57 and <a
 href=http://www.gnu.org/software/automake/automake.html>automake</a>
1.7.
</p>

<h2>Maintainer-mode</h2>

The BOINC configuration system enables an Automake feature
called <b>maintainer-mode</b>.  This is enabled
at <code>configure</code>-time.

<table border=1><tr><th>command-line</th><th>Maintainer-mode?</th><th>Effect</th></tr>
  <tr><td><code>configure</code></td><td>Disabled</td><td>
      If you modify <code>Makefile.am</code>, you need to
      regenerate <code>Makefile.in</code> using <code>automake</code>, and
      your machine-dependent <code>Makefile</code>
      using <code>config.status</code>.
  </td></tr>
  <tr><td nowrap><code>configure --enable-maintainer-mode</code></td>
  <td>Enabled</td><td>
      If you modify <code>Makefile.am</code>, a chain of dependencies
      automatically generates <code>Makefile.in</code>
      and <code>Makefile</code> when you '<code>make</code>'.  This is useful
      if you modify Makefiles a lot but could be annoying if you don't have
      automake installed, have different versions of it among developers, or
      check in <code>Makefile.in</code> to CVS at the same time (in which
      case the timestamp for it will confuse the automatic dependencies).
</td></tr></table>

<h2>Source layout</h2>

<p>
  The top-level <code>Makefile.am</code> contains the
  <code>SUBDIRS=</code> line which sets up directory recursion, and
  the rules for creating source distributions.
</p>
<p>
  Each subdirectory's <code>Makefile.am</code> contains the rules for
  making the binaries and libraries in that directory and any extra
  files to distribute.
</p>
<p>
  Usually you will want to run <code>make</code> from the toplevel
  (the directory containing the file <code>configure</code>), but
  sometimes it is useful to run <code>make</code> and <code>make
    check</code> in certain subdirectories (e.g. <code>client/</code>).
</p>

<h2>Expansion</h2>
If you create a new directory with another <code>Makefile.am</code>,
you should <b>A)</b> make sure the directory is referenced by
a <code>SUBDIRS=</code> line from its
parent <code>Makefile.am</code> and <b>B)</b> add it to the
AC_CONFIG_FILES directive in <code>configure.ac</code>.

<h2>Target machine configure/make</h2>
To compile, use the usual
<pre>
  ./configure
  make
</pre>
<p>
  Example using multiple build directories under a single source
  directory (assuming the same directory is mounted
  on <code>milkyway</code> and <code>shaggy</code>):
  <pre>
    milkyway$ mkdir build
    milkyway$ mkdir build/solaris2.7
    milkyway$ cd build/solaris2.7
    milkyway$ ../../configure
    milkyway$ make

    milkyway$ mkdir build/solaris2.7-gcc3
    milkyway$ cd build/solaris2.7-gcc3
    milkyway$ ../../configure CC=/opt/misc/gcc-3.0.4/bin/gcc CXX=/opt/misc/gcc-3.0.4/bin/g++
    milkyway$ make

    shaggy$ mkdir build/linux
    shaggy$ cd build/linux
    shaggy$ ../../configure
    shaggy$ make
  </pre>
</p>

<h2>Testing</h2>

To test the code:
<pre>
  make check
</pre>

This runs the python tests in the <code>test/</code> directory.  Old PHP-based
tests in <code>test/</code>are also available to be run individually but not
currently maintained.
</p>

<h2>Version number</h2>
To set the BOINC client version number:
<pre>
  set-version 7.17
</pre>
in the BOINC top-level source directory.  This updates
the <code>AC_INIT</code> line in
<code>configure.ac</code> and regenerates files that use the version numbers
(config.h, py/version.py, test/version.inc, client/win/win_config.h, Makefiles)

<h2>Archival/source distribution</h2>
To make source distributions:
<pre>
  make dist
</pre>


<? page_tail(); ?>
