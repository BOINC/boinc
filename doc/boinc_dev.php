<?php
require_once("docutil.php");
page_head("Development and debugging");

echo "

<p>
<ul>
<li> <a href=contact.php>Personnel and contributors</a>
<li> <a href=dev_flow.php>Development information flow</a>
<li> The <a href=http://bbugs.axpr.net/index.php>BOINCzilla bug database</a>.
<li> <a href=email_lists.php>boinc_dev</a>,
an email list for BOINC developers.
<li> <a href=compile.php>Get and compile BOINC software</a>
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
<hr>
Various implementation notes:
<h2>Client</h2>
<ul>
<li> <a href=client_files.php>File structure</a>
<li> <a href=client_fsm.php>FSM structure</a>
<li> <a href=client_data.php>Data structures</a>
<li> <a href=client_logic.php>Main loop logic</a>
<li> <a href=sched.php>Client scheduling policies (new)</a>
<li> <a href=client_sched.php>Client scheduling policies (old)</a>
<li> <a href=host_measure.php>Host measurements</a>
<li> <a href=host_id.php>Host identification</a>
<li> <a href=client_app.php>Core client/application interaction (basic)</a>
<li> <a href=client_app_graphic.php>Core client/application interaction (graphics)</a>
<li> <a href=client_startup.php>Client configuration files</a>
<!--
<li> <a href=disk_management.php>Disk space management</a>
-->
</ul>
<h2>Server programs</h2>
<ul>
<li> <a href=database.php>The BOINC database</a>
<li> <a href=sched_policy.php>Work distribution policy</a>
<li> <a href=backend_state.php>Backend state transitions</a>
<li> <a href=backend_logic.php>The logic of backend programs</a>
<li> <a href=server_debug.php>Debugging server components</a>
</ul>

<h2>Protocols</h2>
<ul>
<li> <a href=comm.php>Protocol overview</a>
<li> <a href=protocol.php>The scheduling server protocol</a>
<li> <a href=rpc_policy.php>Scheduling server timing and retry policies</a>
<li> <a href=upload.php>Data server protocol</a>
<li> <a href=pers_file_xfer.php>Persistent file transfers</a>
</ul>
<h2>Client extensions</h2>
<ul>
<li> <a href=gui_rpc.php>GUI RPCs</a>
    <br> Lets you control or show the status of BOINC clients,
    local or remote.
</ul>
<h2>Web-based extensions (statistics and account management)</h2>
<ul>
<li> <a href=db_dump.php>Downloading statistics data</a>
<li> <a href=cpid.php>Cross-project identification</a>
<li> <a href=web_rpc.php>Web RPCs (possibly useful for statistics sites)</a>
<li> <a href=acct_mgt.php>Account management systems</a>
</ul>
<h2>Miscellaneous</h2>
<ul>
<li> <a href=python.php>Python framework</a>
<li> <a href=prefs_impl.php>Preferences</a>
<li> <a href=trickle_impl.php>Trickle messages</a>
<li> <a href=version_diff.txt>How to see what has changed
between two versions of an executable</a>.
<li> <a href=spec.txt>Spec info for RPMs</a>
</ul>

";
page_tail();
?>

