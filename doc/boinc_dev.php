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
The University of California holds the copyright on all BOINC source code;
by contributing code to BOINC you implicitly assign the copyright
to the University of California.
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
<li> Applications
<ul>
<li> Write an example compound application
(and suggest API revisions to make this easier).
</ul>
<li> Core client:
<ul>
<li> Rewrite the CPU scheduler and work fetch policies
to match the design document (John McLeod is working on this).
<li> After the above is done,
write a simulator for the CPU scheduler and work fetch policies.

<li> Log result start/ends (for use by 3rd-party software like BoincView).

<li> Prevent disk space usage from exceeding user preferences,
and enforce resource shares,
with file deletion according to project policy.
</ul>

<li> BOINC Manager:
<ul>
Change the Statistics tab to use a single graph
with lines of different colors or styles for different projects.

<li> Show progress bars for file transfers and in-progress results.

<li> Show pie charts for disk usage
<li> Sortable columns in Work tab.
</ul>


</ul>
Please check with davea at ssl.berkeley.edu
before undertaking any of these.
";

page_tail();
?>

