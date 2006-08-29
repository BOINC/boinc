<?php
require_once("docutil.php");
page_head("Participating in BOINC projects");
echo "
<h3>General information</h3>
<ul>
<li> <a href=intro_user.php>Getting started</a>
<li> <a href=info.php>Usage rules and privacy policy</a>
<li> <a href=account.php>Choosing and joining projects</a>
<li> <a href=download.php>Download BOINC</a>
<li> Installing BOINC
<ul>
<li> <a href=win_install.php>The Windows installer</a>
<li> <a href=sea.php>Self-extracting archive</a>
<li> <a href=bare_core.php>Core client executable</a>
</ul>

<li> Running BOINC
<ul>
<li> <a href=manager.php>The BOINC manager</a>
<li> <a href=screensaver.php>The BOINC screensaver</a>
<li> <a href=client_unix.php>Core client options and configuration</a>
</ul>

<li> <a href=prefs.php>Preferences</a>
<li> <a href=energy.php>Heat and energy considerations</a>
<li> <a href=host_id.php>Host identification and merging</a>
<li> <a href=multiple_projects.php>Participating in multiple projects</a>
<li> <a href=credit.php>Computation credit</a>
<li> <a href=cpu_sched.php>CPU scheduling</a>
<li> <a href=teams.php>Teams</a>
<li> <a href=anonymous_platform.php>Make your own client software</a>
    <br>Run BOINC on uncommon platforms,
        or inspect the source code before you run it.
<li> <a href=acct_mgrs.php>Account managers</a>
<li> <a href=http://www.cs.wisc.edu/condor/manual/v6.7/3_13Setting_Up.html#SECTION004138000000000000000>BOINC as a Condor backfill job</a>
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
