<?php
require_once("docutil.php");
page_head("Participating in BOINC projects");
echo "
<b>Running BOINC</b>
<ul>
<li> <a href=intro_user.php>Getting started</a>
<li> <a href=projects.php>Choosing projects</a>
<li> <a href=info.php>Usage rules and privacy policy</a>
<li> <a href=host_requirements.php>System requirements</a>
<li> <a href=account.php>Joining a project</a>
<li> Installing and running the BOINC client
<ul>
<li> <a href=client_windows.php>Windows GUI</a>
<li> <a href=client_unix.php>Command-line version (Unix, Mac OS/X)</a>
<li> <a href=service.php>Windows service</a>
</ul>
<li> <a href=prefs.php>Preferences</a>
<li> <a href=host_id.php>Host identification and merging</a>
<li> <a href=startup.php>Participating in multiple projects</a>
<li> <a href=credit.php>Computation credit</a>
<li> <a href=teams.php>Teams</a>
<li> <a href=anonymous_platform.php>Compiling BOINC software yourself</a>
</ul>
<p>
<b>Graphical User Interfaces (GUIs)</b>
<ul>
<li> <a href=gui_rpc.php>Framework for separate GUIs</a>
<li> <a href=gui_rpc_control.php>Access control</a>
<li> <a href=guis.php>Separate GUIs</a>
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
