<?php
require_once("docutil.php");

page_head("Email lists");


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
<a href=http://setiathome.berkeley.edu/forum_help_desk.php>SETI@home message boards</a>
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
    "Summaries of BOINC CVS checkins are posted to this list.
    No other posts, please."
);
list_item("<a href=http://lists.ssl.berkeley.edu/mailman/listinfo/boinc_opt>boinc_opt</a>",
    "For people porting and optimizing BOINC applications."
);
list_item("<a href=http://lists.ssl.berkeley.edu/mailman/listinfo/boinc_helpers>boinc_helpers</a>",
    "For BOINC <a href=help.php>Help Volunteers</a>,
    to discuss policies and user problems."
);
list_item("<a href=http://groups.google.com/group/boinc-team-founders>BOINC team founders</a> (Google group)",
    "Discussion of team-related issues."
);
list_end();

echo "
<h2>Problems with email lists</h2>
<p>
Dec 2 2005: some people are reporting that their postings
to these email lists never appear.
Here's a reply from our system administrator:
<blockquote>
My first guess is that the barracuda antispam system is matching their email address or email sending server to a DNSBL (blacklist) and dropping the message. We use several popular DNSBL's to either block or tag blatant spam (spamcop, spamhaus, etc.) The lists we use are usually pretty good at only listing blatant spammers, but sometimes valid addresses are listed for short periods until they can get delisted. This typically happens when a spammer is using an ISP with DHCP addresses -- the spammer gets caught, the dhcp address goes to someone else and that person gets screwed.

<p>
We could disable antispam filtering on your lists, but I wouldn't advise it as you'd get a lot more of spam. For instance, according to my count for the period 11/13 - 12/1, boinc_alpha list got around 58 valid messages, while the antispam system blocked another 85 messages. It's your call.

</blockquote>

";

page_tail();

?>
