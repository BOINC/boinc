<?php
require_once("docutil.php");

page_head("Community and resources");

echo "
<h2>Participants</h2>
<p>
To ask questions, or to report bugs in the BOINC client software,
please go to the
<a href=http://setiweb.ssl.berkeley.edu/sah/forum_help_desk.php>Message
board area</a> of SETI@home.

<p>
There are
<a href=links.php>web sites for BOINC participants</a>
in various languages.

<p>
See also
<a href=http://setiweb.ssl.berkeley.edu/sah/download_network.php>GUIs and add-on software for BOINC</a>.

<h2>Email lists</h2>

The follow email lists are available.
Click to subscribe or post to a list.
";
list_start();
list_item(
    "<a href=http://ssl.berkeley.edu/mailman/listinfo/boinc_projects>boinc_projects</a>",
    "For people operating BOINC projects.
    Use it to ask questions, report bugs, or request enhancements to
    the BOINC server software.
    Announcements of modifications and upgrades to BOINC will posted here.
");
list_item("<a href=http://ssl.berkeley.edu/mailman/listinfo/boinc_dev>boinc_dev</a>",
    "For people developing, debugging or porting the BOINC software.
");
list_item("<a href=http://ssl.berkeley.edu/mailman/listinfo/boinc_loc>boinc_loc</a>",
    "For people localizing, or translating the BOINC software.
");
list_item(
    "<a href=http://www.ssl.berkeley.edu/mailman/listinfo/boinc_stats>boinc_stats</a>",
    "For people developing web sites showing statistics for BOINC projects."
);
list_item("<a href=http://ssl.berkeley.edu/mailman/listinfo/boinc_cvs>boinc_cvs</a>",
    "Summaries of BOINC CVS checkins are posted to this list.
    No other posts, please."
);
list_item("<a href=http://www.ssl.berkeley.edu/mailman/listinfo/boinc_opt>boinc_opt</a>",
    "For people porting and optimizing BOINC applications."
);
list_end();
echo "
<h2>Getting source code</h2>
<p>
At any given point there are two different versions
of the BOINC source code (maintained as separate CVS projects):
<ul>
<li> <b>boinc</b> is the development version.
It may be unstable, and it may not be compatible with public BOINC projects.
<li> <b>boinc_public</b> is the stable version.
It generally is the version being used
(on both client and server) by public BOINC projects.
</ul>
You can get the BOINC source code in several ways:
<ul>
<li>
<a href=source/>Download</a> a tarball or .zip file
(generated nightly).
<li>
Browse the CVS repository via a
<a href=http://boinc.berkeley.edu/cgi-bin/cvsweb.cgi/>web-based interface</a>.
<li>
Access the CVS repository directly, e.g. with a command like
<pre>
cvs -d :pserver:anonymous@alien.ssl.berkeley.edu:/home/cvs/cvsroot checkout boinc
<pre>
</ul>
<p>
Source code for a typical BOINC application,
SETI@home, is <a href=http://boinc.berkeley.edu/seti_source/>here</a>.
<h2>Bug database</h2>
<p>
You can browse the BOINC
<a href=http://setiathome.ssl.berkeley.edu/taskbase/database.cgi>bug-tracking database</a>.

<h2>News feeds</h2>
<p>
RSS-based news feeds are available from BOINC
and from several BOINC-based projects:
<ul>
<li> BOINC: http://boinc.berkeley.edu/rss_main.php
<li> SETI@home: http://setiweb.berkeley.edu/sah/rss_main.php
</ul>

<h2>Graphics</h2>
<p>
The BOINC logo uses the Planet Benson font from
<a href=http://www.larabiefonts.com>Larabie Fonts</a>.
Hi-res logos:
<a href=logo.png>PNG</a>, <a href=logo.jpg>JPEG</a>,
<a href=logo.gif>GIFF</a>.

<p>
Check out this
<a href=hatfield.png>BOINC logo</a>
by Jared Hatfield, a high school student.

<h2>Other</h2>
A good summary of distributed computing projects,
including those based on BOINC, is at
<a href=http://www.aspenleaf.com/distributed/>www.aspenleaf.com</a>.

";

page_tail();
?>
