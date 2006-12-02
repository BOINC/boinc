<?php
require_once("docutil.php");

page_head("Be a Help Volunteer");

echo "
<h2>Why is volunteer support needed?</h2>
<p>
Volunteer computing is contributing to research in
many areas of science - biology, medicine,
environmental study, physics, chemistry, astronomy, and others.
The more computing power is available,
the faster these research projects can reach their objectives,
and the more new projects will be attracted.
<p>
Of the 1 billion PCs in the world today,
only 1 million -  a tenth of a percent -
are participating in volunteer computing.
What about the other 99.9%?
Many of them are owned by nontechnical people,
who use them as web-browsing and email-reading 'appliances'.
There are many reasons why such people would
resist, or have difficulty, participating in volunteer computing:
<ul>
<li>
Many nontechnical people are unfamiliar or uncomfortable with tasks
(such as downloading and installing programs,
typing a URL into a web browser,
and copying and pasting text)
that are routine for experienced computer users.
As a result, the process of installing and using BOINC seems daunting to them.

<li>
Nontechnical people often have (well-founded) concerns about
the security aspects of volunteer computing.
These concerns can often be addressed by explaining
how BOINC prevents the obvious attacks.

<li>
The basic function of BOINC - a single
program that automatically downloads other programs
from the Internet, and switches back and forth between them -
is unusual, and may need to be explained.
</ul>

Web-based support for BOINC - FAQs, message boards, web pages -
is ineffective for many people.
Inevitably, we write in a technical language that not everyone understands.
By providing one-on-one, real-time interactive support,
preferably by voice,
we can assess the knowledge and experience of an individual user,
we can give advice at the right level and in the right language,
and we can make their experience with BOINC successful and enjoyable.
<p>
In this way we can start to reach the remaining 99.9% of
the world's computer users,
and take volunteer computing (and scientific research) to a new level.

<h2>What do you need to do?</h2>
<p>
To be a BOINC Help Volunteer, you must:
<ul>
<li> Be familiar with using BOINC, preferably with several projects.
It helps if you've spent some time answering questions
on the Q&A message boards of a BOINC-based project.

<li> Have a computer with Skype
(preferably with a headset for voice communication,
though text-only is OK too).
Install the current version (2.0 or later) of Skype.
<li> Be available to handle calls on a regular basis
(a few hours a week, or whatever you can spare).
<li> Be willing to answer questions patiently and courteously.
<li> Be willing to spend some additional time researching
problems that you can't solve immediately.
<li> Handle help emails in a timely manner.
</ul>
If this sounds OK, read on!

<h2>Setting up a separate Skype account</h2>
<p>
The Skype ID you use for help calls will be publicly visible,
so you'll want to create a 'Help account'
separate from your existing Skype account.
When you're 'on call', run Skype under your Help account
(you can switch accounts using the File/Sign Out menu).

<p>
<b>IMPORTANT: your Skype 'Help account' must make its status
visible on the web.</b>
To do this: run Skype and log in to your Help account.
<ul>
<li> Windows: Select to Tools/Options menu.
Click on Privacy.
Check the box labeled 'Allow my status to be shown on the web'.

<li> Mac: go to Skype/Preferences.
Select the 'privacy' tab.
Check the box labeled 'Allow my status to be shown on the web'.
</ul>

<p>
What if you want to accept calls from either your regular
Skype ID or your Help ID?
It's possible - you need to run two Skypes at the same time,
one under each ID.
On Windows, the two copies of Skype need to run as different users;
here's how to do it:
<ul>
<li> Go to Control Panel / User Accounts.
Click 'Create a new account'.
Name it 'BOINC Help' or whatever you want.
Make it of type 'Computer administrator'.
Click the icon for the new account,
click 'Create a password', and give it a password.
Close the dialog.
<li>
Right-click on a Skype icon.
Select 'Run as...'.
Select 'The following user',
choose 'BOINC Help', and enter the password.
<li>
A new copy of Skype will appear.
Log in to your Help account,
or create a new Skype account if needed.
</ul>

<p>
You may also want to create a new email account for help emails
(your help email address will not be made public,
but users will be able to post to it through a web interface).

<h2>Handling calls</h2>
For each call:
<ul>
<li> Tell the customer your name, and get their name.
<li> Find out what they need -
information about BOINC or about a particular project,
hand-holding while installing BOINC,
or help resolving a problem.

<li> If they're calling with a problem, get the background info:
what kind of computer, what version of the OS,
what kind of Internet connection.
If they already have BOINC,
find out whether they've actually installed it
and what version it is.
Jot this all down.

<li> If they're running an old version of BOINC,
have them install the current version.

<li> Ask them to describe the problem step-by-step.
Walk them through the problem scenario if possible.

<li> If they become frustrated, angry or abusive,
try to remain calm and polite.
Suggest that they contact another volunteer,
or consult message boards for help.
As a last resort, just hang up.
You can 'block' a user using Contacts/Advanced/Manage Blocked Users.

<li> Stay 'on task' - don't get sucked into solving general PC problems.
There may be other BOINC help calls waiting.
</ul>
After you've solved their problem,
take the opportunity to inform the caller on topics such as:
<ul>
<li> The value of participating in more than one project
(tell them about one of the projects other than SETI@home).
<li> How to get their friends and family interested in BOINC.
</ul>

<h2>What to do if you can't solve a problem</h2>
<p>
If you get a call with a problem that's completely outside your sphere,
tell the caller that,
and suggest that they call another volunteer
or try the message boards.
<p>
If you get a call with a problem that you think you can solve
but need to do some research,
tell the caller that.
Ask them to send you an email using the
web page they're looking at,
and to include their email address so that you can get back to them.
Then research the problem, using
<ul>
<li> Your own trial and error;
<li> Research on the BOINC web site or message boards;
<li> Post to the boinc_helpers@ssl.berkeley.edu email list;
<li> If the above fail, post to the boinc_dev@ssl.berkeley.edu email list.
</ul>
Reply to the caller with the results of your inquiry,
preferably withing 2-3 days.

<h2>OK?  Let's get started</h2>

<p>
<b>Please read everything above here carefully.</b>
If you've done so and want to continue, then
<ul>
<li> Create a Skype account for handling Help calls
<li> <a href=help_vol_edit.php>Create a Help Volunteer account</a>.
</ul>
";
page_tail();
?>
