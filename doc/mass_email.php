<?php
require_once("docutil.php");
page_head("Sending mass emails");
echo "
Sending periodic mass emails is a good way to retain participants.
BOINC provides a script <b>html/ops/mass_email_script.php</b>
for sending mass emails.
This script lets you personalize messages,
lets you send multipart HTML/text messages,
and manages checkpoint/restart when dealing with large
numbers of participants.
<p>
The script requires that you use
<a href=Http://phpmailer.sourceforge.net/>PHPMailer</a>,
is a PHP function for sending mail that's more full-featured
than the one built into PHP.
Download it, put it in html/inc, and set some variables in
<a href=http://boinc.berkeley.edu/web_config.php>your project.inc file</a>.
<p>
Participants are categorized as follows:
<ul>
<li> <b>Failed</b>: zero total credit.
These people failed to download and install the client software,
or failed to get it working (e.g. because of proxy problems)
or uninstalled it before finishing any work.
<li> <b>Lapsed</b>: nonzero total credit but recent average credit < 1:
These people did work in the past, but none recently.
<li> <b>Current</b>: recent average credit >= 1.
These are your active user.
</ul>

To use mass_email_script.php, create the following
files in your html/ops directory:
";
list_start();
list_item("email_failed_html",
    "HTML message sent to failed users.  Example: ".html_text(
"<html>
<body bgcolor=ffffcc>
Dear <name/>:
<p>
Test Project continues to do pioneering computational research
in the field of Submandibular Morphology.
In recent months we have discovered over 17 new varieties
of Frombats.
<p>
Our records show that you created a Test Project account
on <create_time/> but that your computer hasn't completed any work.
Quite possibly you encountered problems installing or using
the software.
Many of these problems have now been fixed,
and we encourage you to visit
<a href=http://a.b.c>our web site</a>,
download the latest version of the software, and try again.
<p>
<font size=-2>
To avoid receiving future emails from Test Project,
<a href=<opt_out_url/>>click here</a>.
</font>
</td></tr></table>
</body>
</html>"
)."
");
list_item("email_failed_text", "Text message sent to failed users.
    Example: ".html_text("
Dear <name/>:

Test Project continues to do pioneering computational research
in the field of Submandibular Morphology.
In recent months we have discovered over 17 new varieties of Frombats.

Our records show that you created a Test Project account
on <create_time/> but that your computer hasn't completed any work.
Quite possibly you encountered problems installing or using the software.
Many of these problems have now been fixed,
and we encourage you to visit <a href=http://a.b.c>our web site</a>,
download the latest version of the software, and try again.

To avoid receiving future emails from Test Project, visit
<opt_out_url/>"
)."
");
list_item("email_failed_subject", "Subject line sent to failed users.
    Example: 'Test Project News'.
");
list_item("email_lapsed_html", "HTML message sent to lapsed users");
list_item("email_lapsed_text", "Text message sent to lapsed users");
list_item("email_lapsed_subject", "Subject line sent to lapsed users");
list_item("email_current_html", "HTML message sent to current users");
list_item("email_current_text", "Text message sent to current users");
list_item("email_current_subject", "Subject line sent to current users");
list_end();
echo "
BOINC will replace the following macros in your HTML and text:
";
list_start();
list_item("&lt;name/&gt;", "User name");
list_item("&lt;create_time/&gt;", "When account was created (D M Y)");
list_item("&lt;total_credit/&gt;", "User's total credit");
list_item("&lt;opt_out_url/&gt;",
    "URL for opting out (this URL includes a salted version
of the user's account key, and so is different for every user).
");
list_end();
echo "
NOTE: You should ALWAYS include an 'opt-out' link
at the bottom of emails (both HTML and text).
In may be illegal for you to do a mass email without one.
Make sure you test this link.

<h2>Testing your email</h2>

<p>
Test your email before sending it out to the world.
As distributed, mass_email_script.php has the following
variables defined near the top:
<pre>
// set the following to false to actually send emails
// (rather than just print to stdout)
//
\$testing = true;

// If the following is nonzero, email will be sent to the given user ID
// Otherwise it will be sent to all users
//
\$userid = 1;
</pre>
To start, set \$userid to the ID of your own user record.
Run the script by typing
<pre>
php mass_email_script.php
</pre>
It will print (to stdout) the contents of all three email types
(failed, lapsed, and current).
Verify that the result subject, HTML and text are correct.
<p>
Then set \$testing = false and run the script again.
You'll get three emails; check them.
<p>
Then set \$testing = false and \$userid = 0,
create an empty file called <b>mass_email.log</b> (see below),
and run the script.
You'll get voluminous output to stdout, but no emails will be sent.
Control-C it quickly if you want.
Make sure that each user is being sent the right type of email.
<p>
When you're sure that everything is correct,
set \$testing = false,
set <b>mass_email.log</b> to empty,
and run the script.
It will now send mass emails.
Depending on the size of your user table,
it may take hours or days to complete.
You can control-C it and restart whenever you want;
it automatically picks up where it left off (see below).

<h2>Checkpoint/restart</h2>
<p>
Mails are sent in order of increasing user ID.
The file <b>mass_email.log</b> has a list of IDs that have been processed.
On startup, the script reads this file, finds the last entry,
and starts from there.

<p>
If you are starting a mass email from the beginning,
empty the file <b>mass_email.log</b>; i.e.
<pre>
truncate mass_email.log
</pre>

<h2>Avoiding spam filtering</h2>
Your email is less likely to be rejected by spam filters if:
<ul>
<li> Your HTML and text versions have the same text.
<li> Your HTML version either contains no images,
or has at least 400 words.
</ul>

";
page_tail();
?>
