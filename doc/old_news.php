<?
require_once("docutil.php");
page_head("Archived news");
echo "
<br><br>
<b>July 25, 2003</b>
<br>
We've added two new web-site features for BOINC projects:
user profiles and message boards.
These are visible on the <a href=http://maggie.ssl.berkeley.edu/ap/>beta-test web site</a>.
<br><br>
<b>July 23, 2003</b>
<br>
The <a href=source/>BOINC source code</a> is available again,
under <a href=legal.html>a new public license</a>.
Version 1.05 incorporates bug fixes and new features,
including the ability to add your own graphics to the screensaver.
The beta test has been resumed, and the scheduling server and
database have been moved to a new machine.
<br><br>
<b>June 10, 2003</b>
<br>
<a href=http://boinc.berkeley.edu/db_dump.php>XML based statistics</a> for
the Astropulse beta test are
<a href=http://setiboinc.ssl.berkeley.edu/ap/stats/>available</a>.
<br><br>
<b>May 30, 2003</b>
<br>
No big news to report,
but thanks to the beta test we've fixed a number of bugs,
involving runaway applications, too-large output files,
screensaver behavior, and mechanisms for
quitting and resetting projects.
<br><br>
<b>April 29, 2003</b>
<br>
We have created a second project, allowing beta testers to
experiment with dividing their resources between multiple projects.
<br><br>
<b>April 23, 2003</b>
<br>
We have resumed the BOINC beta test.
<br><br>
<b>April 9, 2003</b>
<br>
Due to a legal issue,
we are suspending the BOINC beta test,
and we have requested that BOINC source code
no longer be available at Sourceforge.net.
We hope to resolve this issue soon.
<br><br>
<b>March 31, 2003</b>
<br>
We are preparing a BOINC-based version of SETI@home.
See a <a href=setiathome.jpg>preview of the graphics</a>.
<br><br>
<b>March 25, 2003</b>
<br>
<a href=http://www.boinc.dk/index.php?page=download_languages>Non-English
language.ini files</a> are available.
Preferences include time-of-day restrictions.
Core client and applications communicate via shared memory and signals
rather than files, reducing disk traffic.
<br><br>
<b>March 19, 2003</b>
<br>
New account parameters and preferences: URL, limit number of processors,
frequency of writes to disk, whether to show your computers on the web.
<br><br>
<b>March 4, 2003</b> 
<br>
Participants can have separate preferences
(such as network and disk usage limits)
for computers at home, work, and school.
<br><br>
<b>February 25, 2003</b>
<br>
Participants can limit their upstream and downstream network bandwidth.
<br><br>
<b>February 22, 2003</b>
<br>
Participants can control the parameters (color, transparency,
timing) of the Astropulse graphics.
<br><br>
<b>February 19, 2003</b>
New feature: secure, verified email address update.
<br><br>
<b>January 29, 2003</b>
<br>
A <a href=http://setiathome.berkeley.edu/~eheien/ap_ss.jpg>screenshot</a> of the
BOINC client running AstroPulse, our first test application.
<br><br>
<b>December 10, 2002</b>
<br>
We have started a beta test of BOINC using
the Astropulse application.
Many bugs have been found and fixed.
<br><br>
<b>August 24, 2002</b>
<br>BOINC is under development.
The basic features are working on UNIX platforms.
We plan to release the first public application of BOINC later this year.
";
page_tail();
?>
