<?
require_once("docutil.php");
page_head("Participating in BOINC projects");
echo "
<b>Running BOINC</b>
<ul>
<li> <a href=intro_user.php>Getting started</a>
<li> <a href=projects.php>Choosing project(s)</a>
<li> <a href=host_requirements.php>System requirements</a>
<li> <a href=account.php>Joining a project</a>
<li> <a href=client.php>Running the BOINC client</a>
<li> <a href=prefs.php>Preferences</a>
<li> <a href=host_id.php>Host identification and merging</a>
<li> <a href=startup.php>Participating in multiple projects</a>
<li> <a href=credit.php>Computation credit</a>
<li> <a href=teams.php>Teams</a>
<li> <a href=anonymous_platform.php>Compiling BOINC software yourself</a>
<li> User-supplied FAQs by
    <a href=http://homepage.mac.com/pauldbuck/>Paul D. Buck</a> and
    <a href=http://users.iafrica.com/c/ch/chrissu/boinc-README.html>Chris Sutton</a>
<li> <a href=http://setiboinc.ssl.berkeley.edu/ap/stats.php>Leader boards</a>
</ul>
<p>
<b>Third-party software and web sites</b>
<ul>
<li> <a href=gui_rpc.php>Framework for separate GUIs</a>
<li> <a href=guis.php>Separate GUIs</a>
<li> <a href=db_dump.php>Downloading statistics data</a>
<li> <a href=proxy_server.php>Proxy servers</a> (why BOINC doesn't have them).
</ul>
";
page_tail();
?>
