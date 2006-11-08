<?php
require_once("docutil.php");

page_head("Getting source code");
echo "

<h2>CVS modules</h2>

The BOINC source code consists of two CVS modules:
<ul>
<li><b>boinc</b> contains the source code for all parts of BOINC itself
(client, server, web, database).
<li><b>boinc_samples</b> contains
<a href=example.php>several example applications</a>
together with Windows and Mac project files and a Linux makefile
for building the applications.
It also includes some libraries (GLUT, jpeglib, etc.)
that many applications will need, but which are not part of BOINC.
Check out this module in the same parent directory as <b>boinc</b>.
</ul>
The modules are accessable as follows:
<p>
Protocol: pserver
<br>
Server: alien.ssl.berkeley.edu
<br>
User: anonymous
<br>
Folder: /home/cvs/cvsroot

<p>
For example, to check out a module on a Unix system, type a command like
<pre>
cvs -d :pserver:anonymous:@alien.ssl.berkeley.edu:/home/cvs/cvsroot checkout boinc
</pre>
On Windows, get a CVS client
(we like <a href=http://www.tortoisecvs.org/>Tortoise CVS</a>),
right-click on the parent directory,
select 'CVS checkout', and fill in the dialog with the above values.
<p>
<h2>Browsing source code via the web</h2>
<p>
You can browse the 
<a href=http://boinc.berkeley.edu/cgi-bin/cvsweb.cgi/boinc/>boinc</a>
or
<a href=http://setiathome.berkeley.edu/cgi-bin/cvsweb.cgi/boinc_samples/>boinc_samples</a> modules
via a web-based interface.
This is useful for getting individual files, or seeing the revision history.

<h2>Source code road map</h2>
<p>
The BOINC source tree includes the following directories:
";
list_start();
list_item("api",
    "The BOINC API (for applications)"
);
list_item("apps",
    "Some test applications."
);
list_item("client",
    "The BOINC core client."
);
list_item("clientgui",
    "The BOINC Manager."
);
list_item("db",
    "The database schema and C++ interface layer."
);
list_item("doc",
    "BOINC documentation (PHP web pages)."
);
list_item("html/ops",
    "PHP files for the operational web interface."
);
list_item("html/user",
    "PHP files for the participant web interface."
);
list_item("html/inc",
    "PHP include files."
);
list_item("html/languages",
    "Translation files."
);
list_item("lib",
    "Code that is shared by more than one component
    (core client, scheduling server, etc.)."
);
list_item("py",
    "Python modules used by tools."
);
list_item("sched",
    "The scheduling server, feeder, and file upload handler."
);
list_item("test",
    "Test scripts."
);
list_item("tools",
    "Operational utility programs."
);
list_item("zip",
    "Compression functions; not used by BOINC,
    but may be useful for applications."
);
list_end();

echo "
<h2>CVS tags</h2>
<p>
<b>We maintain tags for the client software (core client and manager)
in the boinc module.
For other parts of the software (e.g., server and API),
use the current version.
</b>
<dl>
<dt>
stable
<dd>
The latest publicly-released version of the client software,
generally well-tested.
<b>DO NOT USE THE STABLE VERSION OF SERVER AND API SOFTWARE.
IF YOU DO, IT WILL BE FAR OUT OF DATE.
USE THE CURRENT VERSION INSTEAD.</b>
<dt>
staging
<dd>
The version currently being alpha-tested (so at least it compiles).
<dt>
boinc_core_release_x_y_z
<dd>
The source code for version x.y.z.
</dl>
For a list of available tags, go
<a href=http://boinc.berkeley.edu/cgi-bin/cvsweb.cgi/boinc/>here</a>,
scroll to the bottom of the page,
and look at the popup menu after 'tag:'.

<h2>Mac Menubar code</h2>
The source code for the Mac Menubar,
a simple GUI for Mac OS X that we no longer support,
is <a href=menubar>here</a>.
If anyone wants to maintain this, let us know and we'll put it under CVS.

";

page_tail();
?>
