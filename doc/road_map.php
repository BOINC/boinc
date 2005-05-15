<?php
require_once("docutil.php");
page_head("Source code road map");
echo "
<p>
BOINC is distributed via CVS, tarballs compressed with gzip, and Windows
zip files.
<ul>
<li>
If installing from CVS, check out the entire CVS tree.
<li>
If installing on Windows, unzip boinc.zip or boinc.tar.gz,
and unzip win_build.zip inside the BOINC directory.
<li>
If installing on Mac OS X, unzip boinc.zip or boinc.tar.gz,
and unstuff mac_build.sit inside the BOINC directory.
</ul>

<p>
Unpacking the BOINC source code creates the following directories:
<dl>
<dt> RSAEuro/
<dd> An RSA implementation (included for convenience; not covered
under the BOINC public license).
<dt> api/
<dd> The BOINC API (for applications)
<dt> apps/
<dd> Some test applications.
<dt> client/
<dd> The BOINC core client.
<dt> client/win/
<dd> Core client files particular to the Windows GUI version.
<dt> db/
<dd> The database schema and interface functions.
<dt> doc/
<dd> HTML documentation files.
<dt> html_ops/
<dd> PHP files for the operational web interface.
<dt> html_user/
<dd> PHP files for the participant web interface.
<dt> lib/
<dd> Code that is shared by more than one component
(core client, scheduling server, etc.).
<dt> sched/
<dd> The scheduling server, feeder, and file upload handler.
<dt> test/
<dd> Test scripts.
<dt> tools/
<dd> Operational utility programs.
</dl>
<p>
On UNIX systems, the BOINC software (both server and client)
can be built by typing
<pre>
  ./_autosetup  [Only needed if using CVS; not needed if using tarball]
  ./configure
  make
  make install  [Optional: installs libraries and header files useful for building apps]
</pre>
in the top directory. If you want to build the Unix/Mac graphical client, you will need to
install <a href='http://www.wxwidgets.org/'>wxWidgets</a>.

 Automatic tests can be run using:
<pre>
  make check
</pre>
A list of options to the configure script can be found by using
<pre>
./configure --help
</pre>
The most useful of these is
<pre>
./configure --prefix=/path/to/install/headers/and/libraries/
</pre>
Note that if building from CVS you should have fairly recent versions of 
<a href='http://directory.fsf.org/GNU/autoconf.html'>autoconf</a> and <a href='http://directory.fsf.org/GNU/automake.html'>automake</a>
installed.  These are maintained by the <a href='http://www.gnu.org'>GNU project</a>.
";
page_tail();
?>
