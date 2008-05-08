<?php

require_once("../inc/util.inc");

page_head("Instructions for testers");

echo "

<h3>Becoming a tester</h3>
<p>
The BOINC testing program helps us find and fix bugs
in the BOINC software before we release it to the public.
Testing is vital to the success of BOINC.
If you're a tester, then:
<ul>
<li>
You'll need to spend about 1 hour per week doing testing;
<li>
Some of your computer power will be spent running test work,
and you may receive no credit for it.
<li>
You must have enough computer experience to
write clear descriptions of problems.
<li>
You may be asked to assist, via phone or email,
in troubleshooting.
</ul>
If you are OK with all this and want to help test BOINC,
send email to
<a href=http://boinc.berkeley.edu/contact.php>David Anderson or Rom Walton</a>,
and if we need more testers we'll set up an account for you.

<p>
Testers must subscribe to the
<a href=http://www.ssl.berkeley.edu/mailman/listinfo/boinc_alpha>boinc_alpha email list</a>.
(You don't need to be a tester to subscribe to this list).



<h3>When to test</h3>
<p>
Periodically (every couple of weeks, sometimes more)
we'll send an announcement on the email list,
asking alpha testers to download and test a specific new release of BOINC.
These releases can be downloaded from
<a href=http://boinc.berkeley.edu/download_all.php?dev=1>here</a>.
Please try to report test results within two days of receiving the email.

<h3>How to test</h3>
<p>
The client software under test may have bugs that wipe out tasks in progress.
Therefore we recommend that you install the test software
in a different BOINC directory than the one you normally use for BOINC.
<p>
The set of test cases is
<a href=http://boinc.berkeley.edu/test_matrix.php>here</a>.
For each release, please try to do all the General tests.
Please try to do as many of the other tests
as your time and computer environment permit.
<p>
Depending on what you're testing,
you can select which project(s) to attach to:
<ul>
<li> The BOINC Alpha project (this one) has tasks
with large input and output files (4.5 MB each)
and that use about 1 minute of CPU time.
This is useful for testing file upload/download,
and for testing things that happen on task completion.
<b>You don't need to attach to this project to be an Alpha Tester.
Please do not stay attached to this project for long periods</b>.

<li> You can use the Cunning Plan project
(http://isaac.ssl.berkeley.edu/cplan) for tests
that involve creating accounts.

<li> You can use other projects (SETI@home, Einstein@home, CPDN)
for other purposes.
</ul>
<p>
If at any time you experienced problems with BOINC that are not
exercised by any of these tests,
please post to the <a href=http://www.ssl.berkeley.edu/mailman/listinfo/boinc_alpha>boinc_alpha@ssl.berkeley.edu</a> email list.


<h3>How to report test results</h3>
<p>
The preferred way to report test results is through the
<a href=test_form.php>web-based interface</a>.

<p>
You can optionally also submit bug reports to the
<a href=http://boinc.berkeley.edu/trac/wiki/WikiStart>BOINC bug database</a>.
You may also send email to the relevant area owner, as listed
<a href=http://boinc.berkeley.edu/trac/wiki/DevProcess>here</a>.

<p>
If you're not sure whether something is a bug,
post to the boinc_alpha@ssl.berkeley.edu email list.
<p>
Read about
<a href=http://boinc.berkeley.edu/trac/wiki/ReportBugs>Reporting Hard Bugs</a>.


<h3>Resources</h3>
<p>
Windows users: if you have install/uninstall problems,
you may need to use the Windows Installer CleanUp Utility: see
<a href=http://support.microsoft.com/default.aspx?scid=kb;en-us;290301>here</a>
and
<a href=http://windowssdk.msdn.microsoft.com/en-us/library/ms707976.aspx>here</a>
";

page_tail();
?>
