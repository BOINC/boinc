<?php
require_once("docutil.php");
page_head("GUI URLs");
echo "
<p>
<b>GUI URLs</b> is a mechanism that projects to pass URLs to the client,
for display as hyperlinks in the GUI.
These links will be shown
when the project is selected in the <b>Projects</b> tab.
<p>
To use this feature,
include a file 'gui_urls.xml' in the project root directory,
with the following form:

",html_text("
<gui_urls>
    <gui_url>
        <name>Your account</name>
        <description>View your account information and credit totals</description>
        <url>http://foo.project.com/show_user.php?userid=<userid/></url>
    </gui_url>
    <gui_url>
        <name>Help</name>
        <description>Get help about SETI@home</description>
        <url>http://foo.project.com/help.php</url>
    </gui_url>
    <ifteam>
        <gui_url>
            <name>Team</name>
            <description>Info about <team_name/></description>
            <url>http://foo.project.com/team_display.php?teamid=<teamid/></url>
        </gui_url>
    </ifteam>
    ...
</gui_urls>
"),"
<p>
Each entry describes a GUI URL.
These URLs (macro-substituted as described below)
will be sent to client hosts in the reply to scheduler RPCs.
Team-specific entries should be enclosed in
<code>&lt;ifteam></code>;
they will be sent only if the user belongs to a team.
<p>
The components of a <code>&lt;gui_url></code> element are:
";
list_start();
list_item("name", "A short name, used e.g. as a menu item");
list_item("description", "An explanation, used e.g. as a rollover popup");
list_item("url", "The URL");
list_end();
echo "
All items are macro-substituted as follows:
";
list_start();
list_item(htmlspecialchars("<userid/>"), "The user ID");
list_item(htmlspecialchars("<user_name/>"), "The user name");
list_item(htmlspecialchars("<teamid/>"), "The team ID");
list_item(htmlspecialchars("<team_name/>"), "The team name");
list_item(htmlspecialchars("<hostid/>"), "The host ID");
list_item(htmlspecialchars("<authenticator/>"), "The user's account key");
list_end();
page_tail();
?>
