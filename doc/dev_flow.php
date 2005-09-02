<?
require_once("docutil.php");

page_head("Development information flow");

echo "
This page describes the (proposed) structure of the
BOINC development, debugging, and release management processes.
The basic information flow is shown below.
Ovals represent people, rectangles represent information channels.
A, B, C, D and E represent the different BOINC development
areas (Manager, core client, server, etc.;
actually there are more than five).
'Owner A' represents the person who owns area A,
as listed in <a href=contact.php>Personnel</a>.
<p>
BOINCzilla is <a href=http://bbugs.axpr.net/index.php>here</a>.
The email lists are <a href=email_lists.php>here</a>.
BOINC message boards are <a href=dev/>here</a>.

<img src=dev_flow.png>
<p>
<h3>Participants</h3>
<ul>
<li> Report bugs (or learn of workarounds) on the BOINC message boards.
<li> Learn of new releases on the BOINC web site.
<li> Note: we need ways of 'pushing' info to participants, e.g. via the Manager.
</ul>
<h3>Area owners</h3>
<ul>
<li> Reads the relevant BOINC message board on a regular basis.
Decides if new bugs are present.
Adds entries to BOINCzilla.
<li>
Monitors the relevant categories of BOINCzilla.
Manages entries (delete, merge, prioritize, assign).
</ul>
<h3>Developers</h3>
<ul>
<li> Are assigned tasks via BOINCzilla.
</ul>
<h3>Alpha testers</h3>
<ul>
<li> The boinc_alpha email list is used to give instructions,
and for discussion of tests and procedures.
<li> If find bugs, log them in BOINCzilla.
<li> Use web-based interface for submitting test summaries.
</ul>
<h3>Release manager</h3>
<ul>
<li> Decide when to create test releases;
communicate with alpha testers via email list.
<li> Decide when to make public releases,
based on web-based reports
and on contents of BOINCzilla.
</ul>

";
page_tail();
?>
