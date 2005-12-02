<?php
require_once("docutil.php");

page_head("Email lists");


echo "

The follow email lists are available.
Click to subscribe or post to a list.
";
list_start();
list_item(
    "<a href=http://ssl.berkeley.edu/mailman/listinfo/boinc_announce>boinc_announce</a>",
    "Announcements of new versions of BOINC client software."
);
list_item(
    "<a href=http://ssl.berkeley.edu/mailman/listinfo/boinc_projects>boinc_projects</a>",
    "An email list for people operating BOINC projects.  Questions and problems involving BOINC server software.  Announcements of upgrades and changes.
    Do NOT post questions about BOINC client software here.
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
<h2>Problems with email lists</h2>
<p>
Dec 2 2005: some people are reporting that their postings
to these email lists never appear.
Here's a reply from our system admininistrator:
<blockquote>
My first guess is that the barracuda antispam system is matching their email address or email sending server to a RBL (blacklist) and dropping the message. We use several popular RBL's to either block or tag blatant spam (spamcop, spamhaus, etc.) The lists we use are usually pretty good at only listing blatant spammers, but sometimes valid addresses are listed for short periods until they can get delisted. This typically happens when a spammer is using an ISP with DHCP addresses -- the spammer gets caught, the dhcp address goes to someone else and that person gets screwed.

<p>
We could disable antispam filtering on your lists, but I wouldn't advise it as you'd get a lot more of spam. For instance, according to my count for the period 11/13 - 12/1, boinc_alpha list got around 58 valid messages, while the antispam system blocked another 85 messages. It's your call.

</blockquote>

";

page_tail();

?>
