<?php
require_once("docutil.php");

page_head("Account management systems");

echo "
<p>
To create an account with BOINC projects, a participant must:
<ul>
<li> locate BOINC project web sites,
read them, and decide which to join;
<li> Download and install the BOINC client software.
</ul>
and then for each selected project:
<ul>
<li> fill out a web registration form;
<li> handle an email;
<li> cut and paste a URL and account key into the BOINC client.
</ul>

<p>
If the participant chooses N projects,
there are N forms to fill out,
N emails to handle, and N dialog interactions with the BOINC client.
This is tedious if there are lots of projects.
Furthermore, it involves cutting and pasting long random strings,
which is intimidating to some participants.

<p>
This document describes BOINC's support for <b>account management systems</b>,
which streamline the process of finding and joining BOINC projects.
A typical account management system is implemented as a web site.
The participant experience is:
<ul>
<li> Visit the account manager site,
set up a 'meta-account' (name, email, password),
browse a list of projects, and click checkboxes to select projects.
<li> Receive email from each selected project,
and click on a link in the email.
<li> Download and install the BOINC client software from the account manager.
<li> Enter the meta-account name and password in a BOINC client dialog.
</ul>
This requires about 1/3 of the interactions of the manual approach,
and avoids dealing with long random strings.

<h2>Implementation</h2>
<p>
An account management system works as follows:
<br>
<img src=acct_mgt.png>
<br>

<ol>
<li> The participant sets up his meta-account and selects projects.
<li> The account manager issues a <b>create account</b> RPC
to each selected project.
<li> The project creates an account (marked as 'unconfirmed')
and sends an email to the participant.
<li> The participant opens the email and clicks on a link in it,
causing the account to be marked as 'confirmed'.
<li> The account manager periodically polls each selected project
with a <b>query account</b> RPC,
waiting for all accounts to become confirmed.
<li> When all accounts are confirmed,
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

<h2>Remote Procedure Call (RPC) Specifications</h2>
<p>
This architecture involves two RPC mechanisms:
<ul>
<li> <b>Account creation RPCs</b> (steps 2 and 5 above);
<li> <b>Account manager RPCs</b> (step 8 above).
</ul>
The underlying protocol of both mechanisms is as follows:
<ul>
<li> Each RPC is an HTTP GET transaction.
<li> The input is the GET arguments, i.e. a string of the form
".html_text("
param1=val1&param2=val2&...&paramn=valn
")."
where param1 ... paramN are the parameter names,
and val1 ... valn are the values.
<li>
The output is XML.
</ul>

<h2>Account creation RPCs</h2>
<p>

<h3>Create account</h3>
";

list_start();
list_item("URL", "project_url/am_create.php");
list_item(
	"input",
        "email_addr: email address
		<br>
        nonce: nonce ID (a long random string that is hard to guess,
            e.g. in PHP:
            <code>md5(uniqid(rand(), true))</code>
        <br>
        acct_mgr_name: name of the account manager site
");
list_item(
	"output",
    html_text("<am_create_reply>
    [ <error>message</error> ]
    [ <success/>
</am_create_reply>
    ")
);
list_item(
	"action",
		"If the project already has an account with that email address,
        it returns an error.
        Otherwise the project creates a tentative account
		and sends email to the given address, of the form:
        <pre>
The computing project [ project name ] has been requested by [ acct_mgr_name ]
to create an account with email address [ email_addr ].
To confirm, visit the following URL:
    xxx

If you didn't initiate this request, ignore this message.
        </pre>
		When the participant visits xxx, the tentative account is confirmed.
");
list_end();

echo "
<h3>Query account</h3>
";

list_start();
list_item("URL", "project_url/am_query.php");
list_item("input",
    "nonce (the one passed to am_create.php)"
);
list_item("output",
    html_text("<am_query_reply>
    [<error>MSG</error>]
    [ <success/>
    <confirmed>0</confirmed> ]
    [ <success/>
    <account_key>KEY</account_key> ]
</am_query_reply>
    ")
);
list_item("action",
    "If the tentative account has been confirmed,
    attempts to create an account with the previously given email address.
    If this fails (e.g., because an account with that email address
    already exists) returns an error.
    Otherwise returns returns the account key."
);
list_end();

echo "
<h3>Get account info</h3>
";

list_start();
list_item("URL", "project_url/am_get_info.php");
list_item("input", "account_key");
list_item("output",
	html_text("<am_get_info_reply>
    <success/>
    <id>ID</id>
    <name>NAME</name>
    <country>COUNTRY</country>
    <postal_code>POSTAL_CODE</postal_code>
    <global_prefs>
        GLOBAL_PREFS
    </global_prefs>
    <project_prefs>
        PROJECT_PREFS
    </project_prefs>
    <url>URL</url>
    <send_email>SEND_EMAIL</send_email>
    <show_hosts>SHOW_HOSTS</show_hosts>
    <teamid>N</teamid>
</am_get_info_reply>

or

<am_get_info_reply>
    <error>MSG</error>
</am_get_info_reply>
    ")
);
list_item("action", "returns data associated with the given account");
list_end();
echo "
<h3>Set account info</h3>
";
list_start();
list_item("URL", "project_url/am_set_info.php");
list_item("input",
    "account_key
    <br>[ name ]
    <br>[ country ]
    <br>[ postal_code ]
    <br>[ global_prefs ]
    <br>[ project_prefs ]
    <br>[ url ]
    <br>[ send_email ]
    <br>[ show_hosts ]
    <br>[ teamid ]  <i>zero means quit current team, if any</i>
    "
);
list_item("output",
    html_text("<am_set_info_reply>
    [ <error>MSG</error> ]
    [ <success/> ]
</am_set_info_reply>")
);
list_item("action",
    "updates one or more items of data associated with the given account"
);

list_end();

echo "
<h2>Account manager RPCs</h2>

";

list_start();
list_item("URL", "Given in the file <b>acct_manager_url.xml</b>");
list_item("input", "name
    <br>password"
);
list_item("output",
    html_text("<accounts>
    [ <error>MSG</error> ]
    [ <account>
        <url>URL</url>
        <authenticator>KEY</authenticator>
      </account>
      ...
    ]
</accounts>")
);
list_item("action",
    "returns a list of the accounts associated with this meta-account"
);
list_end();

page_tail();
?>
