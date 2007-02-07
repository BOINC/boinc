<?php
require_once("docutil.php");
page_head("BOINC software development ");

if (!$book) {
    echo "
        <ul>
        <li> <a href=#addon>Writing add-on software</a></li>
        <li> <a href=#build>Get and build BOINC software</a></li>
        <li> <a href=#dev>BOINC development</a></li>
        <li> <a href=#projects>Development projects</a></li>
        </ul>
    ";
}

echo "
<a name=addon></a>
<h2>Writing add-on software</h2>
<p>
BOINC's architecture is 'open';
documented interfaces making it possible to
develop various types of applications and web sites that
interact with BOINC components.
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
    "This mechanism lets you write programs for local editing of preferences."
);
list_end();
echo "
<p>
Check the
<a href=addons.php>index of add-on software</a>
before writing anything (it may already exist).
The index has instructions for submitting new add-ons.

<a name=build></a>
<h2>Get and build BOINC software</h2>
    <ul> 
    <li> <a href=source_code.php>Get BOINC source code</a>
    <li> <a href=server.php>Building server software</a>
    <li> <a href=compile_app.php>Building applications</a>
    <li> <a href=compile_client.php>Building BOINC client software</a>
    </ul>

<a name=dev></a>
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
We need volunteers to help with software testing and development.
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
To get started, find a small bug fix or enhancement to do
(look at the BOINC bug database, the email lists, or message boards for ideas).
Look at the source code and think about how you would implement it.
Then communicate with the <a href=contact.php>area owner</a>,
sketching what you want to do and how.
Work the the area owner to carry out and check in the work.

<a name=projects></a>
<h2>Development projects</h2>
<p>
The following medium-to-large development projects are available:
<ul>
<li> Applications
<ul>
<li> Write an example compound application
(and suggest API revisions to make this easier).
</ul>
<li> Core client:
<ul>
<li> Compute disk usage (total, per-project) in a separate thread.
<li> Don't enforce RAM limits unless free RAM is low
<li> Extend general preferences to allow users to
specify different time-of-day restrictions for different days of the week.
<li> 
Write a simulator for the CPU scheduler and work fetch policies
(Derrick Kondo is working on this).

<li> Log result start/ends (for use by 3rd-party software like BoincView).

<li> Prevent disk space usage from exceeding user preferences,
and enforce resource shares,
with file deletion according to project policy.
<li> Make messages of class MSG_USER_ERROR translatable.
</ul>

<li> BOINC Manager:
<ul>
<li> Advanced prefs dialog

<li> Show progress bars for file transfers and in-progress results
(this requires changing the container class from Spreadsheet to Grid).

<li> Sortable columns in the Work tab.
</ul>

<li> Server/Back End:
    <ul>
    <li> When using HR, if the scheduler has sent one result of a WU
        using a particular app version,
        it should use the same app version for other results from that WU.
        Need to change protocol to specify version num;
        need to change client to use this.
    <li> Implement a mechanism so that server
        software detects incompatible database format
    <li> 
        Scheduler: implement mechanisms so that server:
        <ul>
        <li> Sends only results likely to finish by their deadline
        <li> Sends commands to abort results that can't get credit
        <li> Sends commands to recommend abort of results
            that may get credit, but are not useful
            (i.e. canonical result already found)
        <li> Attempts to send results from the same WU to
            hosts with similar speed,
            so that a fast host doesn't have to wait weeks to get credit.
        </ul>
    <li> Implement a 'benchmark result' mechanism:
        every host runs a benchmark result per app version,
        and the CPU time determines credit/CPU for future results
    <li> Use the information sent by the client
        (queued work, fraction done, and deadlines)
        to avoid sending jobs that won't finish by deadline.
    </ul>


</ul>
Please check with <a href=contact.php>David Anderson</a>
before undertaking any of these.
";

page_tail();
?>

