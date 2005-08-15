<?php
require_once("docutil.php");
page_head("Development and debugging");

echo "
<p>
If you do C++ system programming you may be able
to help us maintain and enhance BOINC.
In any case, you are welcome to browse the source code and
give us feedback.
You should understand how BOINC works
(for both <a href=participate.php>participants</a>
and <a href=create_project.php>projects</a>)
before getting into the source code.

<p>
For starters, look through the
<a href=http://bbugs.axpr.net/index.php>BOINCzilla bug database</a>
and try fixing a bug or two.
Sign up for the
<a href=http://boinc.berkeley.edu/community.php#email_lists>boinc_dev</a>
email list to communicate with other BOINC developers.
Also, read:
<ul>
<li> <a href=compile.php>Get and compile BOINC software</a>
<li> <a href=coding.php>BOINC coding style</a>
</ul>

The following medium-to-large development projects are available:
<ul>
<li> BOINC Manager:
Change the Statistics tab to use a single graph
with different colors for different projects.

<li> BOINC Manager:
Use progress bars for file transfers and in-progress results.

<li> BOINC Manager:
Use pie charts for disk usage

<li> Disk space management: prevent disk space usage from
exceeding user preferences,
    and enforce resource shares,
    with file deletion according to project policy.

<li> Core client:  use select() instead of polling
for I/O (RPCs, file transfers, GUI RPCs).

<li> Use database IDs instead of names to identify
results and WUs in the core client
(to increase efficiency in the server,
since indices on integers are apparently more efficient
than indices on text fields).
This involves minor but pervasive changes.

</ul>
Please check with davea at ssl.berkeley.edu
before undertaking any of these.
<hr>
Various implementation notes:
<h2>Core client</h2>
<ul>
<li> <a href=client_files.php>File structure</a>
<li> <a href=client_fsm.php>FSM structure</a>
<li> <a href=client_data.php>Data structures</a>
<li> <a href=client_logic.php>Main loop logic</a>
<li> <a href=sched.php>Client scheduling policies (new)</a>
<li> <a href=client_sched.php>Client scheduling policies (old)</a>
<li> <a href=client_debug.php>Debugging</a>
<li> <a href=host_measure.php>Host measurements</a>
<li> <a href=host_id.php>Host identification</a>
<li> <a href=client_app.php>Core client/application interaction (basic)</a>
<li> <a href=client_app_graphic.php>Core client/application interaction (graphics)</a>
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
<h2>Miscellaneous</h2>
<ul>
<li> <a href=python.php>Python framework</a>
<li> <a href=prefs_impl.php>Preferences</a>
<li> <a href=trickle_impl.php>Trickle messages</a>
<li> <a href=version_diff.txt>How to see what has changed
between two versions of an executable</a>.
<li> <a href=acct_mgt_new.php>Account management systems</a>
<li> <a href=spec.txt>Spec info for RPMs</a>
<li> <a href=stats_summary.php>Statistics summaries</a>
</ul>

";
page_tail();
?>

