<?php
require_once("docutil.php");
page_head("Participating in BOINC projects");
echo "
<b>Running BOINC</b>
<ul>
<li> <a href=intro_user.php>Getting started</a>
<li> <a href=projects.php>Choosing projects</a>
<li> <a href=info.php>Usage rules and privacy policy</a>
<li> <a href=account.php>Joining a project</a>
<li> <a href=download.php>Download client software</a>
<li> Installing and running the BOINC client
<ul>
<li> <a href=manager.php>The BOINC manager</a>
<li> <a href=win_install.php>The Windows installer</a>
<li> <a href=client_unix.php>Command-line version (Unix, Mac OS/X)</a>
<li> <a href=client_mac.php>Installing BOINC on Mac OS/X</a>
</ul>
<li> <a href=prefs.php>Preferences</a>
<li> <a href=host_id.php>Host identification and merging</a>
<li> <a href=multiple_projects.php>Participating in multiple projects</a>
<li> <a href=credit.php>Computation credit</a>
<li> <a href=cpu_sched.php>CPU scheduling</a>
<li> <a href=teams.php>Teams</a>
<li> <a href=anonymous_platform.php>Compiling BOINC software</a>
    <br>Go here if you want to run BOINC on unusual platforms,
        or if you want to inspect the source code before
        you run it.
</ul>
<p>
<b>Graphical User Interfaces (GUIs)</b>
<ul>
<li> <a href=gui_rpc.php>Framework for separate GUIs</a>
<li> <a href=gui_rpc_control.php>Access control</a>
<li> <a href=http://setiweb.ssl.berkeley.edu/sah/download_network.php>Separate GUIs</a>
</ul>
<p>
<b>Statistics web sites</b>
<ul>
<li> <a href=db_dump.php>Downloading statistics data</a>
<li> <a href=cpid.php>Cross-project identification</a>
</ul>
<b>Third-party software and web sites</b>
<ul>
<li> <a href=proxy_server.php>Proxy servers</a> (why BOINC doesn't have them).
</ul>
";
page_tail();
?>
