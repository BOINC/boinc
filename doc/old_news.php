<?
require_once("docutil.php");
page_head("Archived news");
echo "
<br><br>
<b>October 30, 2003</b>
<br>
New minor versions of the core client and beta-test apps have been released.
XML statistics data for the beta test
<a href=http://setiboinc.ssl.berkeley.edu/ap/stats.php>is available</a>;
it's intended for graphical or tabular representation.
<br><br>
<b>October 18, 2003</b>
<br>
A SETI@home application has been added to the beta test.
The scheduling server detect results that can't be sent
to any hosts, and flags them.
HTTP 404 errors on downloads are treated as unrecoverable.
Graphics can be limited by frame rate or percent of CPU.
Don't send two results from the same workunit to a single user.
Added JPEG support to application graphics.
Added \"confirm before accept executable\" mechanism.

<br><br>
<b>September 11, 2003</b>
<br>
BOINC is discussed in
<a href=http://www.nytimes.com/2003/09/11/technology/circuits/11dist.html>
an article by Joan Oleck in the New York TImes</a>.
<br><br>

<b>September 4, 2003</b>
<br>
We've released BOINC version 2.0.
All BOINC components must be upgraded to major version 2.
We've upgraded the <a
href=http://maggie.ssl.Berkeley.edu/ap/>Astropulse</a> server to 2.01 and
released <a href=http://setiboinc.ssl.Berkeley.edu/ap/download.php>BOINC core
client 2.01</a>.
This was due to an architecture and protocol change in
specifying resource estimates and limits.
<br><br>
<b>September 1, 2003</b>
<br>
BOINC server tools are now Python-based.  A <a href=python.php>database
back-end API</a> allows quick development of data-manipulation utilities.
<br><br>
<b>August 15, 2003</b>
<br>
We rewrote parts of the scheduling server architecture to make operations
more flexible and efficient.
The <code>timeout_check</code>
daemon has been replaced by a more general <code>transitioner</code> daemon.
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
