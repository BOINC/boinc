<?php
require_once("docutil.php");

page_head("Getting source code");
echo "

<h2>SVN modules</h2>

The BOINC source code consists of two directories:
<ul>
<li><b>boinc</b> contains the source code for all parts of BOINC itself
(client, server, web, database).
<li><b>boinc_samples</b> contains
<a href=example.php>several example applications</a>
together with Windows and Mac project files and a Linux makefile
for building the applications.
It also includes Windows versions of some libraries (GLUT, jpeglib, etc.)
that many applications will need, but which are not part of BOINC.
Check out this module in the same parent directory as <b>boinc</b>.
</ul>
The modules are accessible as follows:
<pre>
svn co http://boinc.berkeley.edu/svn/trunk/boinc
svn co http://boinc.berkeley.edu/svn/trunk/boinc_samples
</pre>
On Windows, get a SVN client like Tortoise SVN.
Right-click on the parent directory,
select 'SVN checkout', and fill in the dialog with the above URL.
<p>
<h2>Browsing source code via the web</h2>
<p>
You can browse the boinc or boinc_samples code via
<a href=http://boinc.berkeley.edu/trac/>a web-based interface</a>.
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
<h2>SVN tags</h2>
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
IF YOU DO, IT MAY BE FAR OUT OF DATE.
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
To check out a branch (for example, boinc_core_release_5_8a)
use the following URL:

<pre>
http://boinc.berkeley.edu/svn/branches/boinc_core_release_5_8/boinc
</pre>


";

page_tail();
?>
