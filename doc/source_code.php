<?php
require_once("docutil.php");

page_head("Getting source code");
echo "

<h2>CVS repositories</h2>

The BOINC source code is maintained two CVS modules:
<ul>
<li><b>boinc</b> contains the source code for all parts of BOINC itself
(client, server, web, database).
<li><b>boinc_samples</b> contains a sample BOINC application, <b>upper_case</b>,
together with Windows and Mac project files and a Linux makefile
for building the application.
It also includes some bits of code (GLUT, jpeglib, etc.)
that many applications will need, but which are not part of BOINC.

</ul>
If you are developing an application,
you should check out these modules in the same parent directory.

<h2>CVS tags</h2>
<p>
In the <b>boinc</b> module,
the trunk is the development version;
it may not compile or run.
Other versions are tagged as follows:
<dl>
<dt>
stable
<dd>
The latest publicly-released version,
generally well-tested.
<dt>
staging
<dd>
The version currently being alpha-tested
(so at least it compiles).
<dt>
boinc_core_release_x_y_z
<dd>
The source code for version x.y.z.
</dl>
For a list of available tags, go
<a href=http://boinc.berkeley.edu/cgi-bin/cvsweb.cgi/boinc/>here</a>,
scroll to the bottom of the page,
and look at the popup menu after 'tag:'.

<h2>Source code</h2>
You can get the BOINC source code in two ways:
<ul>
<li>
Access the CVS repository directly, e.g. with a command like
<pre>
cvs -d :pserver:anonymous:@alien.ssl.berkeley.edu:/home/cvs/cvsroot checkout boinc
</pre>
to get the development version, or
<pre>
cvs -d :pserver:anonymous:@alien.ssl.berkeley.edu:/home/cvs/cvsroot checkout -r stable boinc
</pre>
to get the stable version.
<p>
<li>
Browse the 
<a href=http://boinc.berkeley.edu/cgi-bin/cvsweb.cgi/boinc/>boinc</a>
or
<a href=http://setiathome.berkeley.edu/cgi-bin/cvsweb.cgi/boinc_samples/>boinc_samples</a> modules
via a web-based interface.
This is useful for getting individual files, or seeing the revision history.

</ul>

";

page_tail();
?>
