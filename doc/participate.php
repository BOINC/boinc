<?php
require_once("docutil.php");
page_head("Participating in BOINC projects");
echo "
<h3>Beginners</h3>
<p>
BOINC lets you use your PC to help scientific research.
BOINC is a 'platform' that is used by a number of <i>projects</i>,
located at various universities around the world,
and doing research in a variety of areas.
<br><center>
<img src=projects.png>
</center><br>
Each project has its own web site.
You can do work for multiple projects.
    <a href=help.php>Talk or chat with a BOINC volunteer</a>
        for information about BOINC or for help installing BOINC.
<h3>Intermediate users</h3>
<ul>
<li> <a href=intro_user.php>Getting started</a>
<li> <a href=info.php>Usage rules and privacy policy</a>
<li> <a href=account.php>Choosing and joining projects</a>
<li> Installing BOINC
<ul>
<li> <a href=win_install.php>The Windows installer</a>
<li> <a href=mac_advanced.php>The Macintosh installer</a>
<li> <a href=sea.php>Self-extracting archive</a>
<li> <a href=bare_core.php>Core client executable</a>
</ul>

<li> Running BOINC
<ul>
<li> <a href=manager.php>The BOINC manager</a>
<li> <a href=screensaver.php>The BOINC screensaver</a>
</ul>

<li> <a href=prefs.php>Preferences</a>
<li> <a href=addons.php>Add-on software</a>
<li> <a href=energy.php>Heat and energy considerations</a>
<li> <a href=host_id.php>Host identification and merging</a>
<li> <a href=multiple_projects.php>Participating in multiple projects</a>
<li> <a href=credit.php>Computation credit</a>
<li> <a href=teams.php>Teams</a>
<li> <a href=acct_mgrs.php>Account managers</a>
</ul>
<h3>Advanced users</h3>
<ul>
<li> <a href=client_unix.php>Core client options and configuration</a>
<li> <a href=anonymous_platform.php>Make your own client software</a>
    <br>Run BOINC on uncommon platforms,
        or inspect the source code before you run it.
<li> <a href=http://www.cs.wisc.edu/condor/manual/v6.7/3_13Setting_Up.html#SECTION004138000000000000000>BOINC as a Condor backfill job</a>
<li> <a href=proxy_server.php>Proxy servers</a> (why BOINC doesn't have them).
<li> <a href=cpu_sched.php>CPU scheduling</a>
<li> <a href=manager_skin.php>Creating a skin for the BOINC Manager</a>
</ul>
<p>
<h3>Related projects</h3>
If you're interested in donating your time (rather than computing power),
check out
<ul>
<li> <a href=http://clickworkers.arc.nasa.gov/>Clickworkers</a>
(from NASA Ames Research Center)
<li> <a href=http://stardustathome.ssl.berkeley.edu/>Stardust@home</a>
(from UC Berkeley Space Sciences Lab).
<li> <a href=http://www.pgdp.net/c/default.php>Distributed Proofreaders</a>
</ul>

";
page_tail();
?>
