<?php
require_once("docutil.php");

page_head("Account management systems");

echo "
<p>
To create an account with BOINC projects, a participant must:
<ul>
<li> Locate BOINC project web sites, e.g. using Google.
<li> Read the web sites, and decide which to join;
<li> Download and install the BOINC client software.
</ul>
and then for each selected project:
<ul>
<li> Go through the Attach to Project wizard.
</ul>

<p>
If the participant chooses N projects,
there are N web sites to visit and N Wizards to complete.
This is tedious if there are lots of projects.

<p>
This document describes BOINC's support for <b>account management systems</b>,
which streamline the process of finding and joining BOINC projects.
A typical account management system is implemented as a web site.
The participant experience is:
<ul>
<li> Visit the account manager site,
set up a 'meta-account' (name, email, password),
browse a list of projects, and click checkboxes to select projects.
<li> Download and install the BOINC client software from the account manager.
<li> Enter the meta-account name and password in a BOINC client dialog.
</ul>
This requires many fewer interactions than the manual approach.

<h2>Implementation</h2>
<p>
An account management system works as follows:
<br>
<img src=acct_mgt2.png>
<br>

<ol>
<li> The participant sets up his meta-account and selects projects.
<li> The account manager issues a <b>create account</b> RPC
to each selected project.
<li> 
the participant downloads and installs the BOINC client software
from the account manager.
The install package includes a file
(specific to the account manager)
containing the URL of the account manager.
<li> The BOINC client runs, and asks the participant to enter
the name and password of his meta-account.
<li> The BOINC client does a <b>query accounts</b> RPC
to the account manager, obtaining a list of accounts.
It then attaches to these accounts and proceeds.
</ol>

The <b>create account</b> RPCs
are described <a href=web_rpc.php>here</a>.

<h2>Core client functionality</h2>
<p>
The BOINC core client uses the following files to
keep track of account manager information.
<dl>
<dt>
<b>acct_mgr_url.xml</b>
<dd>
This file identifies the account manager.
It is typically bundled with the BOINC client in
an installer package.
Its format is:
".html_text("
<acct_mgr>
    <name>Name of BOINC account management system</name>
    <url>http://acctmgr.com/</url>
</acct_mgr>
")."

<p>
Note: the URL is that of the account manager's web site.

<dt>
<b>acct_mgr_login.xml</b>
<dd>
This file contains meta-account information.
Its format is:
".html_text("
<acct_mgr_login>
   <login>name</login>
   <password>xxx</password>
</acct_mgr_login>
")."
</dl>
If the core client finds acct_mgr_url.xml but not acct_mgr_login.xml,
it prompts for a name and password,
stores them in acct_mgr_login.xml,
and makes an account manager RPC.
The core client offers menu items for making an account manager RPC,
and for changing the name/password.

<h2>Account manager RPC</h2>

The core client contacts the account manager
using an HTTP POST request.
";

list_start();
list_item("URL", "<b>BASE_URL/rpc.php</b>, where BASE_URL is the URL
        of the account manager web site.");
list_item("input", html_text("
<acct_mgr_request>
    <name>John</name>
    <password>xxx</password>
    <host_cpid>b11ddc5f36c9a86ff093c96e6930646a</host_cpid>
    <client_version>5.3.2</client_version>
    <run_mode>auto</run_mode>
    <project>
       <url>http://setiathome.berkeley.edu/</url>
       <project_name>SETI@home</project_name>
       <suspended_via_gui>0</suspended_via_gui>
    </project>
    ...
</acct_mgr_request>
")
);
list_item("output",
    html_text("<acct_mgr_reply>
    <name>Account Manager Name</name>
    [ <error>MSG</error> ]
    [ <repeat_sec>xxx</repeat_sec> ]
    [ 
      <account>
         <url>URL</url>
         <authenticator>KEY</authenticator>
         [ <detach/> ]
      </account>
        ...
    ]
</acct_mgr_reply>")
);
list_item("action",
    "Returns a list of the accounts associated with this meta-account.
    The 'host_cpid' argument identifies the host.
    To make it comparable with the host CPID in stats files,
    the value MD5(host_cpid+email_addr) is passed.
    <p>
    Optionally returns commands to detach projects.
    <p>
    Optionally returns a time interval after which another RPC should be done.
    <p>
    NOTE: the XML must be as above, with the &lt;url>
    and &lt;authenticator> elements on a single line,
    and the &lt;account> and &lt;/account> tags
    on separate lines."
);
list_end();

page_tail();
?>
