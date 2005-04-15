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
<li> <a href=sea.php>Self-extracting archive (UNIX></a>
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
<li> <a href=links.php>Web sites related to BOINC</a>
</ul>
<p>
<b>Add-on software</b>
<ul>
<li> <a href=download_network.php>Add-on software</a>
<li> <a href=gui_rpc.php>GUI RPCs</a>
    <br> Lets you write programs
    that control or display the status of BOINC clients,
    even across a network.
<li> <a href=proxy_server.php>Proxy servers</a> (why BOINC doesn't have them).
</ul>
<p>
<b>Statistics web sites</b>
<ul>
<li> <a href=stats_sites.php>BOINC statistics sites</a>
<li> <a href=db_dump.php>Downloading statistics data</a>
<li> <a href=cpid.php>Cross-project identification</a>
<li> <a href=acct_mgt.php>Web services (possibly useful for statistics sites)</a>
</ul>
";
page_tail();
?>
