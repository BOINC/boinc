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
<a href=download_network.php>GUIs and add-on software for BOINC</a>.

<a name=email_lists>
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
    "For people doing non-English translations
    of the BOINC GUI or web interfaces.
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

<h2>Translating</h2>
If you speak a non-English language, you can help by
<a href=translation.php>translating the BOINC manager and web pages</a>.

<h2>News feeds</h2>
<p>
RSS-based news feeds are available from BOINC
and from several BOINC-based projects:
<ul>
<li> BOINC: http://boinc.berkeley.edu/rss_main.php
<li> SETI@home: http://setiweb.berkeley.edu/sah/rss_main.php
</ul>

<h2>Logo and graphics</h2>
<p>
The BOINC logo uses the Planet Benson font from
<a href=http://www.larabiefonts.com>Larabie Fonts</a>.
Hi-res versions of the logo:
<a href=logo.png>PNG</a>, <a href=logo.jpg>JPEG</a>,
<a href=logo.gif>GIFF</a>.

<p>
We like the BOINC logo but not fanatically,
and we welcome alternative ideas.
For example, Michael Peele contributed
<a href=NewBOiNC.gif>this</a>,
<a href=BOiNC3.jpg>this</a>,
and <a href=BOiNC2.png>this</a>.
Yopi at sympatico.ca contributed
<a href=boincb6.png>this</a> and
<a href=boincb7.png>this</a>.
If you have an opinion, please contact
<a href=contact.php>David Anderson</a>.

<p>
Banners for BOINC projects from Anthony Hern:
<a href=images/hern_logo3.gif>with</a> and
<a href=images/hern_logo4.gif>without</a> the BOINC logo.
<p>
The 'B in a circle' icon was designed by Tim Lan.
The Mac variant was contributed by Juho Viitasalo.
<p>
Check out this imaginative
<a href=hatfield.png>BOINC graphic</a>
by high school student Jared Hatfield.

<h2>Other</h2>
A good summary of distributed computing projects,
including those based on BOINC, is at
<a href=http://www.aspenleaf.com/distributed/>www.aspenleaf.com</a>.

";

page_tail();
?>
