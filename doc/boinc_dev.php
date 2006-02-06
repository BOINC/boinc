<?php
require_once("docutil.php");
page_head("The BOINC software development process");

echo "

<p>
<ul>
<li> <a href=contact.php>Personnel and contributors</a>
<li> <a href=dev_flow.php>Development information flow</a>
<li> The <a href=http://bbugs.axpr.net/index.php>BOINCzilla bug database</a>.
<li> <a href=email_lists.php>boinc_dev</a>,
an email list for BOINC developers.
<li> <a href=compile.php>Get and compile BOINC software</a>
<li> <a href=impl_notes.php>Implementation notes</a>
<li> <a href=coding.php>BOINC coding style</a>
</ul>
<h2>Getting involved</h2>
<p>
BOINC is free software, distributed under the Lesser GNU Public License (LGPL).
We are in constant need of volunteers to
help with software testing and development.
If you have one or more of the relevant technical skills
(C++ system programming, PHP/MySQL web development,
WxWidgets programming, autoconf/automake expertise, etc.)
you may be able to help us maintain and enhance BOINC.
In any case, you are welcome to browse the source code and give us feedback.
You should understand how BOINC works
(for both <a href=participate.php>participants</a>
and <a href=create_project.php>projects</a>)
before getting into the source code.

<p>
To get started, look at the BOINC bug database, fix a bug or two,
and send your patches to the appropriate area owner.
The following medium-to-large development projects are available:
<ul>
<li> Replace db_base.py with <a href=http://sqlobject.org/>SQLObject</a>.
<li> BOINC Manager:
Change the Statistics tab to use a single graph
with lines of different colors or styles for different projects.

<li> BOINC Manager:
Show progress bars for file transfers and in-progress results.

<li> BOINC Manager:
Use pie charts for disk usage

<li> Show when new versions of the core client and/or BOINC Manager
are available.
Could show in status line of Manager,
as a balloon, or in Messages.

<li> BOINC Manager: sortable columns in Work tab.

<li> Support local editing of preferences
(could be done in the Manager or a separate app).

<li> Core client: write a log file of result start/ends.
(for use by 3rd-party software like BoincView).

<li> Disk space management: prevent disk space usage from
exceeding user preferences,
    and enforce resource shares,
    with file deletion according to project policy.

</ul>
Please check with davea at ssl.berkeley.edu
before undertaking any of these.
";

page_tail();
?>

