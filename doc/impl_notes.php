<?php

require_once("docutil.php");
page_head("Implementation notes");
echo "
<h2>Client</h2>
<ul>
<li> <a href=client_files.php>File structure</a>
<li> <a href=client_fsm.php>FSM structure</a>
<li> <a href=client_data.php>Data structures</a>
<li> <a href=client_logic.php>Main loop logic</a>
<li> <a href=sched.php>Client scheduling policies</a>
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
<li> <a href=stats.php>Credit statistics data</a>
<li> <a href=cpid.php>Cross-project identification</a>
<li> <a href=web_rpc.php>Web RPCs (possibly useful for statistics sites)</a>
<li> <a href=acct_mgt.php>Account management systems</a>
</ul>
<h2>Miscellaneous</h2>
<ul>
<li> <a href=loc_sim/>BOINC simulator</a> (simulate large-scale projects)
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
