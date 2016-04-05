<?php
require_once("docutil.php");

page_head("BOINC email lists");


echo "

The follow email lists are available.
Click to subscribe or post to a list.
Because of spam problems,
you must subscribe to a list in order to post to it.
Make sure you post from the same email address
under which you subscribed.

<p>
<b><font size=+1 color=#f00000>
Note:

These email lists do not provide tech support
for SETI@home or other BOINC projects.
Help for SETI@home is available on the
<a href=https://setiathome.berkeley.edu/forum_help_desk.php>SETI@home message boards</a>
and help for BOINC is available <a href=help.php>here</a>.

</font></b>
";
list_start();
//list_item(
//    "<a href=http://lists.ssl.berkeley.edu/mailman/listinfo/boinc_announce>boinc_announce</a>",
//    "Announcements of new versions of BOINC client software."
//);
list_item(
    "<a href=http://lists.ssl.berkeley.edu/mailman/listinfo/boinc_projects>boinc_projects</a>",
    "For people developing and operating BOINC projects.
    Questions and problems involving BOINC API and server software.
    Announcements of upgrades and changes.
");
list_item("<a href=http://lists.ssl.berkeley.edu/mailman/listinfo/boinc_dev>boinc_dev</a>",
    "For people developing, debugging or porting the BOINC software
    (client, server, and Web).
    Do NOT post questions about how to use the software.
");
list_item("<a href=http://lists.ssl.berkeley.edu/mailman/listinfo/boinc_loc>boinc_loc</a>",
    "For people doing non-English translations
    of the BOINC GUI or web interfaces.
");
list_item(
    "<a href=http://lists.ssl.berkeley.edu/mailman/listinfo/boinc_stats>boinc_stats</a>",
    "For people developing web sites showing statistics for BOINC projects."
);
list_item("<a href=http://lists.ssl.berkeley.edu/mailman/listinfo/boinc_cvs>boinc_cvs</a>",
    "Summaries of changes to the BOINC source code are posted to this list.
    No other posts, please."
);
list_item("<a href=http://lists.ssl.berkeley.edu/mailman/listinfo/boinc_opt>boinc_opt</a>",
    "For people porting and optimizing BOINC applications."
);
list_item("<a href=http://lists.ssl.berkeley.edu/mailman/listinfo/boinc_helpers>boinc_helpers</a>",
    "For BOINC <a href=help.php>Help Volunteers</a>,
    to discuss policies and user problems."
);
list_item("<a href=https://groups.google.com/forum/#!forum/boinc-team-founders>BOINC team founders</a> (Google group)",
    "Discussion of team-related issues."
);
list_end();

page_tail();

?>
