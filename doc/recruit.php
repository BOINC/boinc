<?php
require_once("docutil.php");
page_head("Recruiting and retaining volunteers");
echo "
<h3>Contents</h3>
<ul>
<li> <a href=#web>Project web site</a>
<li> <a href=#pr>Publicity</a>
<li> <a href=#email>Email-based mechanisms</a>
    <ul>
    <li> <a href=#newsletter>Newsletters</a>
    <li> <a href=#reminder>Reminders</a>
    <li> <a href=#f2f>Friend-to-friend</a>
    </ul>
</ul>

<hr>
The following is a list of suggestions for
getting more people to participate in your project.
Much of this is based on
<a href=poll_results.php>the results of the BOINC user survey</a>;
study this yourself.

<p>
Most of these suggestions involve writing
prose to be read by the general public.
If (like many scientists) you are not good at this or avoid doing it,
find someone who is good at it.
This could be one of your students,
a friend of a friend, or a professional writer.

<p>
English is the most widely-spoken language among BOINC participants,
and you should probably use it as the main language for
web materials and email.
BOINC provides mechanisms for <a href=translation.php>Web site translation</a>;
it's generally easy to get volunteers to do this.

<a name=web></a>
<h2>Project web site</h2>

Your project's web site has a large role in attracting participants.
Some suggestions:
<ul>
<li> Present your project's credentials:
the educational credentials of its leaders,
its research track record,
and the status of its institution.
<li> Describe what your project is doing:
its high-level scientific goals, its methods,
the details of the computation being done using volunteers,
and the (non-distributed) computations that precede and follow this.
How will your research affect the lives of everyday people
now and/or 50 years from now?
<li> Who owns the intellectual property that arises from
volunteer computations?
Will it be released to the public?
When, and under what terms?
<li> Show all the scientific results of the computation so far,
and any publications that arise from these results.
(<a href=http://depts.washington.edu/~bakerpg/publications.html>Rosetta@home</a>
and <a href=http://folding.stanford.edu/papers.html>Folding@home</a>
provide good examples of this).
Announce new results and publications on the News column.
Make sure your News column is being properly published as an RSS feed.
<li> Give some personal information about your team members:
their names, background, interests, and preferably a photograph.
This will 'humanize' your project in the eyes of potential participants.
<li> Take an active role in your web site's message boards.
Read them frequently,
and respond quickly to any negative threads that arise.
Make a periodic posting giving 'insider info' on your project.
<li> Make sure your the web site has clear navigation,
so that the above information is easy to find from the front page.
Do a user study - show your web site to a strangers,
ask them to browse it and/or to find particular information,
and get their feedback (you may be surprised).
<li> If possible, create a graphical identity
(logo, color scheme, etc.) for your project.
Your web site should project professionalism and 
inspire confidence and interest in prospective volunteers.


</ul>

<a name=pr></a>
<h2>Publicity</h2>

The world will not beat a path to your door.
You need to work hard to spread the word about your project.

<ul>
<li> Get in the mass media (newspapers, magazines, radio, television)
as much as possible.
If your institution has a PR director or media spokesperson,
contact them while you're developing your project,
and again any time your project has major news.
If no such person is available,
call local media outlets yourself.
<li>
Exploit existing organizational relationships.
If you work at a University,
try to get your project running on the PCs in the teaching labs,
and on the PCs of students, faculty and staff.
If you have connections with organizations with PR capabilities
(i.e., web sites or newsletters),
enlist their support for your project, and get them to publicize it.
A typical example: professional organization in your subject area.
<li>
The BOINC web site will generally announce new projects.
Also, make sure your project is listed on
<a href=acct_mgrs.php>account managers</a> like GridRepublic and BAM!
<li>
Use the web.
Announce your project in forums like Slashdot,
and on the message boards of the major cross-project teams
like BOINC Synergy, Overclockers UK, Team Anandtech, etc.

</ul>

<a name=email></a>
<h2>Email-based mechanisms</h2>
<p>
BOINC provides PHP-based tools
for sending three types of email to participants:
<ul>
<li> <b>Newsletters</b>.
    These are periodic (perhaps every few months) and are sent to all participants.
    Typically you would use them to summarize your project's results,
    to discuss its future plans, to make announcements, etc.
<li> <b>Reminders</b>.
    These are sent to participants who seem to have stopped
    computing to your project, or who signed up but never got any credit.
    Typically they would be short messages,
    encouraging the participant to take a specific action.
<li> <b>Friend-to-friend</b>.
    These are sent by participants to their friends and family,
    to tell them about your project and urge them to join.
    The sender can add an optional message.
</ul>
Effective use of all types of email
is critical to maintaining and growing your participant base.
In the absence of any email,
participation typically decreases by a few percent every month.
<b>BOINC supplies the framework, but you must write the
actual emails, or modify BOINC's samples as needed for your project.  </b>.
<p>
The newsletter and reminder scripts provide the following features:
<ul>
<li> They let you send different emails to different 'classes' of participants.
For example, you can send a different newsletter
to participants who haven't computed for your project in a while.
<li> They let you personalize emails,
    e.g. by inserting the participant's name or their total credit.
<li> They provide a mechanism for inserting a
secure 'opt-out' link.
Note: You should ALWAYS include an 'opt-out' link
at the bottom of emails (both HTML and text).
In may be illegal for you to do a mass email without one.
Make sure you test this link.
</ul>
<p>
The scripts requires that you use
<a href=http://phpmailer.sourceforge.net/>PHPMailer</a>,
is a PHP function for sending mail that's more full-featured
than the one built into PHP.
Download it, put it in html/inc, and set the
USE_PHPMAILER, PHPMAILER_HOST, and PHPMAILER_MAILER variables in
<a href=http://boinc.berkeley.edu/web_config.php>your project.inc file</a>.
<p>
All of the tools let you send multipart HTML/text messages.
We recommend that you use this feature -
and HTML message can include your logo and/or institutional insignia,
can include hyperlinks,
and can look more attractive.

<p>
The general procedure for using each scripts is:
<ul>
<li> Create a directory (mass_email, reminder_email, or ffmail)
    in your html/ops/ directory.
    In that directory, create separate files for
    the text body template, HTML body template, and subject line
    to be sent to each class of participants.
    NOTE: the HTML files are optional;
    if you leave them out, text-only emails will be sent.
<li> 
    Run the script in testing mode (see below)
    to ensure that the emails are as you intend.
<li>
    Once testing is complete, run the script in production mode.
    Typically, the newsletter script is run from the command line.
    The reminder script is typically run as a
    <a href=tasks.php>periodic task</a>, every 24 hours or so.
</ul>

The newsletter and reminder scripts use the recent-average credit
(expavg_credit) field in the user table.
To make sure this value is accurate,
run <a href=project_tasks.php>update_stats</a> manually
if you're not running it as a periodic task.

<h3>Personalizing emails</h3>
<p>
The newsletter and reminder scripts
replace the following macros in your email bodies
(both HTML and text):
";
list_start();
list_item("&lt;name/&gt;", "User name");
list_item("&lt;create_time/&gt;", "When account was created (D M Y)");
list_item("&lt;total_credit/&gt;", "User's total credit");
list_item("&lt;opt_out_url/&gt;",
    "URL for opting out (this URL includes a salted version
of the participant's account key, and so is different for every participant).
");
list_item("&lt;lapsed_interval/&gt;",
    "The number of days since user's client contacted server
    (defined only for lapsed users, see below)."
);
list_item("&lt;user_id/&gt;",
    "The user ID (use this to form URLs)"
);
list_end();

echo "
<p>

<h3>Avoiding spam filtering</h3>
Your email is less likely to be rejected by spam filters if:
<ul>
<li> Your HTML and text versions have the same text.
<li> Your HTML version either contains no images,
or has at least 400 words.
</ul>

<a name=newsletter></a>
<h2>Newsletters</h2>
<p>
The script <b>html/ops/mass_email_script.php</b>
is for sending email newsletters.
The script categorize participants as follows:
<ul>
<li> <b>Failed</b>: zero total credit.
These people failed to download and install the client software,
or failed to get it working (e.g. because of proxy problems)
or uninstalled it before finishing any work.
<li> <b>Lapsed</b>: nonzero total credit but recent average credit < 1:
These people did work in the past, but none recently.
<li> <b>Current</b>: recent average credit >= 1.
These are your active participants.
</ul>

To use the script, create the following files in html/ops/mass_email:
";
list_start();
list_item("failed_html",
    "HTML message sent to failed users.  Example: ".html_text(
"<html>
<body bgcolor=ffffcc>
Dear <name/>:
<p>
Test Project continues to do pioneering computational research
in the field of Submandibular Morphology.
In recent months we have discovered over
17 new varieties of Frombats.
<p>
Our records show that you created a Test Project account
on <create_time/> but that your computer
hasn't completed any work.
Possibly you encountered problems
installing or using the software.
Many of these problems have now been fixed,
and we encourage you to visit
<a href=http://a.b.c>our web site</a>,
download the latest version of the software, and try again.
<p>
<font size=-2>
To not receive future emails from Test Project,
<a href=<opt_out_url/>>click here</a>.
</font>
</td></tr></table>
</body>
</html>"
)."
");
list_item("failed_text", "Text message sent to failed users.
    Example: ".html_text("
Dear <name/>:

Test Project continues to do pioneering computational research
in the field of Submandibular Morphology.
In recent months we have discovered over
17 new varieties of Frombats.

Our records show that you created a Test Project account
on <create_time/> but that your computer
hasn't completed any work.
Quite possibly you encountered problems
installing or using the software.
Many of these problems have now been fixed,
and we encourage you to visit
<a href=http://a.b.c>our web site</a>,
download the latest version of the software, and try again.

To not receive future emails from Test Project, visit
<opt_out_url/>"
)."
");
list_item("email_failed_subject", "Subject line sent to failed users.
    Example: 'Test Project News'.
");
list_item("lapsed_html", "HTML message sent to lapsed users");
list_item("lapsed_text", "Text message sent to lapsed users");
list_item("lapsed_subject", "Subject line sent to lapsed users");
list_item("current_html", "HTML message sent to current users");
list_item("current_text", "Text message sent to current users");
list_item("current_subject", "Subject line sent to current users");
list_end();
echo "
<h3>Testing</h3>

<p>
Test your email before sending it out to the world.
As distributed, mass_email_script.php has the following
variables defined near the top:
<pre>
\$testing = true;
</pre>
Set it to false to actually send emails (rather than just print to stdout).
<pre>
\$userid = 1;
</pre>
If this is nonzero, email will be sent to the given user ID;
Otherwise it will be sent to all users.
<p>
To start, set \$userid to the ID of your own user record.
Run the script by typing
<pre>
php mass_email_script.php
</pre>
It will print (to stdout) the contents of all three email types
(failed, lapsed, and current).
Verify that the subject, HTML and text are correct.
<p>
Then set \$testing = false and run the script again.
You'll get three emails; check them.
<p>
Then set \$testing = false and \$userid = 0,
create an empty file called <b>mass_email/log</b> (see below),
and run the script.
You'll get voluminous output to stdout, but no emails will be sent.
Control-C it quickly if you want.
Make sure that each user is being sent the right type of email.
<p>
When you're sure that everything is correct,
set \$testing = false,
set <b>mass_email/log</b> to empty,
and run the script.
It will now send mass emails.
Depending on the size of your user table,
it may take hours or days to complete.
You can control-C it and restart whenever you want;
it automatically picks up where it left off (see below).

<h3>Checkpoint/restart</h3>
<p>
<b>mass_email_script.php</b>
manages checkpoint/restart when dealing with large numbers of participants.
Mails are sent in order of increasing user ID.
The file <b>mass_email/log</b> has a list of IDs that have been processed.
On startup, the script reads this file, finds the last entry,
and starts from there.

<p>
If you are starting a mass email from the beginning,
empty the file <b>mass_email/log</b>; i.e.
<pre>
truncate mass_email/log
</pre>

<a name=reminder></a>
<h2>Reminder emails</h2>

<p>
The script <b>html/ops/remind.php</b> is for sending reminder emails.
The script categorizes users as follows.
<ul>
<li> <b>Failed</b>: the account was created at least 14 days ago,
    has zero total credit,
    and hasn't received a reminder email in 30 days.
    These people typically either had a technical glitch,
    or their hardware and/or preferences didn't allow sending them work,
    or the application crashed on their host.
    The reminder email should direct them to a web page
    that helps them fix these problems.
<li> <b>Lapsed</b>: the user has positive total credit,
    hasn't done a scheduler RPC in the past 60 days,
    and hasn't been sent a reminder email in the past 30 days.
    They probably stopped running BOINC
    or detached this project.
    The reminder email should gently prod them to
    start running BOINC and attach to this project again.
</ul> 
The numbers 14, 30, and 60 are all parameters in the script;
edit it to change them.
<p>
To use the script, create the following files in <b>html/ops/reminder_email/</b>:
";
list_start();
list_item("failed_html", "HTML message sent to failed users");
list_item("failed_text", "Text message sent to failed users");
list_item("failed_subject", "Subject line sent to failed users");
list_item("lapsed_html", "HTML message sent to lapsed users");
list_item("lapsed_text", "Text message sent to lapsed users");
list_item("lapsed_subject", "Subject line sent to lapsed users");
list_end();
echo "
<p>
remind.php can be run as often as you like.
We recommend running it every hours, specifying it as a task in config.xml.
When it sends email to a user, it stores the time in their database record,
and won't send them another email for at least 30 days.
For this reason, it has no checkpoint/restart mechanism.

<p>
The procedure for testing your reminder email is
similar to that for email newsletters (see above).

<a name=f2f></a>
<h2>Friend-to-friend emails</h2>
<p>
The web page ffemail_form.php lets users send emails to their friends.
To use this feature, you must create the following files in
<b>html/ops/ffmail/</b>:
";
list_start();
list_item("subject", "The subject line used for friend-to-friend emails");
list_item("html", "HTML template for friend-to-friend emails");
list_item("text", "Text template for friend-to-friend emails");
list_end();
echo "
Samples are supplied for each of these.
The following macros are substituted in the message bodies:
";
list_start();
list_item("&lt;fromname/&gt;", "The name of the sender");
list_item("&lt;toname/&gt;", "The name of the recipient");
list_item("&lt;comment/&gt;", "The comment supplied by the sender");
list_end();
page_tail();
?>
