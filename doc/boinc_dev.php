<?php
require_once("docutil.php");
page_head("BOINC software development ");

echo "
<h2>Writing add-on software</h2>
<p>
BOINC's architecture is 'open';
documented interfaces making it possible to
develop various types of applications and web sites that
interact with BOINC's core components.
Examples include:
";
list_start();
list_item("<a href=gui_rpc.php>Client GUIs</a>",
    "Application that control a local or remote core client."
);
list_item("<a href=stats.php>Credit statistics web sites</a>",
    "Web sites showing credit information
    (project totals, user/team/country leaderboards)
    based on daily XML feeds of statistics data from BOINC projects."
);
list_item("<a href=acct_mgt.php>Account managers</a>",
    "Web sites that let BOINC users create and manage accounts
    on multiple projects."
);
list_item("<a href=server_status.php>Server status web sites</a>",
    "Web sites showing the server status of BOINC projects."
);
list_item("<a href=web_rpc.php>Web RPCs</a>",
    "These interfaces let a program or web site
    obtain information about users and hosts from projects."
);
list_item("<a href=prefs_override.php>Local editing of preferences</a>",
    "This mechanism lets you write a program local editing of preferences."
);
list_end();
echo "
<p>
Check the
<a href=addons.php>index of add-on software</a>
before writing anything (it may already exist).
The index has instructions for submitting new add-ons.

<h2>Get and build BOINC software</h2>
    <ul> 
    <li> <a href=source_code.php>Get BOINC source code</a>
    <li> <a href=server.php>Building server software</a>
    <li> <a href=compile_app.php>Building applications</a>
    <li> <a href=compile_client.php>Building BOINC client software</a>
    </ul>

<h2>BOINC development</h2>
<p>
<ul>
<li> <a href=contact.php>Personnel and contributors</a>
<li> <a href=dev_flow.php>Development information flow</a>
<li> The <a href=http://bbugs.axpr.net/index.php>BOINCzilla bug database</a>.
<li> <a href=email_lists.php>boinc_dev</a>,
an email list for BOINC developers.
<li> <a href=impl_notes.php>Implementation notes</a>
<li> <a href=coding.php>BOINC coding style</a>
<li> <a href=test.php>The BOINC testing framework</a>
</ul>
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
<li> Extend general preferences to allow users to
specify different time-of-day restrictions for different days of the week.
<li> 
Write a simulator for the CPU scheduler and work fetch policies
(Derrick Kondo is working on this).

<li> Log result start/ends (for use by 3rd-party software like BoincView).

<li> Prevent disk space usage from exceeding user preferences,
and enforce resource shares,
with file deletion according to project policy.
</ul>

<li> BOINC Manager:
<ul>
Change the Statistics tab to use a single graph
with lines of different colors or styles for different projects.

<li> Show progress bars for file transfers and in-progress results
(this requires changing the container class from Spreadsheet to Grid).

<li> Sortable columns in the Work tab.
</ul>


</ul>
Please check with <a href=contact.php>David Anderson</a>
before undertaking any of these.
";

page_tail();
?>

