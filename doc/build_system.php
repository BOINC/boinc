<?php
require_once("docutil.php");
page_head("The BOINC build system");

echo "
See the <a href=build.php>Software Prerequisites</a>.

<p>
  The BOINC build system uses <a
  href=http://www.gnu.org/software/autoconf/>autoconf</a> 2.57 and <a
 href=http://www.gnu.org/software/automake/automake.html>automake</a>
1.7.

<h2>Maintainer-mode</h2>

The BOINC configuration system enables an Automake feature
called <b>maintainer-mode</b>.  This is enabled
at <code>configure</code>-time.
";
list_start();
list_heading("command line", "Maintainer mode?", "Effect");
list_item("<code>configure</code>", "Disabled",
      "If you modify <code>Makefile.am</code>, you need to
      regenerate <code>Makefile.in</code> using <code>automake</code>, and
      your machine-dependent <code>Makefile</code>
      using <code>config.status</code>.  (The <code>boinc/_autosetup</code>
      script takes care of all of these; run this script every time you modify
      a makefile.)"
);
list_item("<nobr><code>configure --enable-maintainer-mode</code></nobr>",
  "Enabled",
  "If you modify <code>Makefile.am</code>, a chain of dependencies
      automatically generates <code>Makefile.in</code>
      and <code>Makefile</code> when you '<code>make</code>'.  This is useful
      if you modify Makefiles a lot but could be annoying if you don't have
      automake installed, have different versions of it among developers, or
      check in <code>Makefile.in</code> to CVS at the same time (in which
      case the timestamp for it will confuse the automatic dependencies)."
);

list_end();
echo "
<h2>Source layout</h2>

<p>
  The top-level <code>Makefile.am</code> contains the
  <code>SUBDIRS=</code> line which sets up directory recursion, and
  the rules for creating source distributions.
<p>
  Each subdirectory's <code>Makefile.am</code> contains the rules for
  making the binaries and libraries in that directory and any extra
  files to distribute.
<p>
  Usually you will want to run <code>make</code> from the toplevel
  (the directory containing the file <code>configure</code>), but
  sometimes it is useful to run <code>make</code> and <code>make
    check</code> in certain subdirectories (e.g. <code>client/</code>).

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
  gmake
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
    milkyway$ gmake

    milkyway$ mkdir build/solaris2.7-gcc3
    milkyway$ cd build/solaris2.7-gcc3
    milkyway$ ../../configure CC=/opt/misc/gcc-3.0.4/bin/gcc CXX=/opt/misc/gcc-3.0.4/bin/g++
    milkyway$ gmake

    shaggy$ mkdir build/linux
    shaggy$ cd build/linux
    shaggy$ ../../configure
    shaggy$ gmake
  </pre>

<h2>Testing</h2>

To test the code:
<pre>
  gmake check
</pre>

This runs the python tests in the <code>test/</code> directory.  Old PHP-based
tests in <code>test/</code>are also available to be run individually but not
currently maintained.

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
  gmake dist
</pre>

<h2>Strict warning</h2>
To compile BOINC with strict compiler warnings, use
<pre>
./configure CXXFLAGS=\"-Wall -W -Wmissing-prototypes -Wstrict-prototypes -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -fno-common -Wnested-externs\"
</pre>
";
page_tail();
?>
