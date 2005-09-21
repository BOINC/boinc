<?php
require_once("docutil.php");
page_head("Client configuration files");
echo "
<p>
BOINC provides mechanisms for making installers that
configure the client to
contact a particular account manager or project
the first time the Manager is run.
These mechanisms work by having the installer
create <b>configuration files</b> in the BOINC directory.
There are several possible configuration files:
";

list_start();
list_item("acct_mgr_url.xml",
    "Contains the name and URL of an account manager;
    the format is described <a href=acct_mgt.php>here</a>."
);
list_item("acct_mgr_login.xml",
    "Contains credentials (name/password) for the account
    manager identified in acct_mgr_url.xml.
    the format is described <a href=acct_mgt.php>here</a>."
);
$x = html_text(
"<project_init>
    <url>PROJECT_URL</url>
    <name>PROJECT_NAME</name>
    [ <account_key>KEY</account_key> ]
</project_init>
");
list_item("project_init.xml",
    "Specifies a project and optionally an account.
    The format is $x"
);
list_end();
echo "
<p>
The process of making
an installer with a particular configuration file
depends on the platform:
<ul>
<li> Windows: requires InstallShield or other
installer utility.  Contact Rom Walton for details.
<li> Macintosh: Contact Charlie Fenton for details.
<li> Unix: modify the Makefile that generates self-extracting archives.
Should be easy to do yourself.
</ul>
<p>
The startup logic of the BOINC manager is as follows:
<pre>
if the core client is not attached to any projects
    if acct_mgr_url.xml exists
        if acct_mgr_login.xml exists
            issue acct mgr RPC
            if new projects
                show projects attached
            else
                show 'go to site and pick projects' dialog
        else
            jump into acct mgr wizard at get_project_config stage
    else if project_init.xml exists
        jump into attach project wizard at get_project_config stage
        if have account_key
            skip prompt for name/password
            go to attach_project RPC
    else
        start attach_project wizard
        if URL is that of an acct mgr,
        go to acct mgr wizard
</pre>
";

page_tail();

?>
