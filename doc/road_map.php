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
<dd> The BOINC API (parts of which are compiled
into the core client).
<dt> apps/
<dd> Some test applications.
<dt> client/
<dd> The BOINC core client.
<dt> client/win/
<dd> Core client files particular to the Windows GUI version.
<dt> client/mac/
<dd> Core client files particular to the Mac GUI version.
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
<dt> sched_fcgi/
<dd> Separate directory for compiling the server
and file upload handler as Fast CGI programs.
<dt> test/
<dd> Test scripts.
<dt> tools/
<dd> Operational utility programs.
</dl>
<p>
On UNIX systems, the BOINC software (both server and client)
can be built by typing
<pre>
  ./configure
  make
</pre>
in the top directory. Automatic tests can be run using
<pre>
  make check
</pre>
";
page_tail();
?>
