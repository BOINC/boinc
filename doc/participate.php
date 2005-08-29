<?php
require_once("docutil.php");
page_head("Participating in BOINC projects");
echo "
<h3>Running BOINC</h3>
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
<li> <a href=sea.php>Self-extracting archive (Unix/Linux)</a>
<li> <a href=screensaver.php>The BOINC screensaver</a>
</ul>
<li> <a href=prefs.php>Preferences</a>
<li> <a href=host_id.php>Host identification and merging</a>
<li> <a href=multiple_projects.php>Participating in multiple projects</a>
<li> <a href=credit.php>Computation credit</a>
<li> <a href=cpu_sched.php>CPU scheduling</a>
<li> <a href=teams.php>Teams</a>
<li> <a href=anonymous_platform.php>Make your own client software</a>
    <br>Run BOINC on unusual platforms,
        or inspect the source code before you run it.
<li> <a href=acct_mgrs.php>Account managers</a>
</ul>
<p>
<h3>Web sites</h3>
<ul>
<li> <a href=links.php>Web sites related to BOINC</a>
<li> <a href=stats_sites.php>BOINC statistics sites</a>
</ul>
<p>
<h3>Add-on software</h3>
<ul>
<li> <a href=download_network.php>Add-on software</a>
<li> <a href=proxy_server.php>Proxy servers</a> (why BOINC doesn't have them).
</ul>
";
page_tail();
?>
