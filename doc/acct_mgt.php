<?php
require_once("docutil.php");

page_head("Account management systems");

echo "
<p>
The 'manual' procedure for creating BOINC accounts is as follows.
A participant must:
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
<li> Handle an email from each selected project
(click on a link in the email).
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
(specific to this account manager)
containing the URL of the account manager.
<li> The BOINC client runs, and asks the participant to enter
the name and password of his meta-account.
<li> The BOINC client does a <b>query accounts</b> RPC
to the account manager, obtaining a list of accounts.
It then attaches to these accounts and proceeds.
</ol>
<p>
This architecture involves two RPC mechanisms:
<ul>
<li> <b>Account creation RPCs</b> (steps 2 and 5 above);
<li> A <b>Account manager RPCs</b> (step 8 above).
</ul>
This document describes these two RPC mechanisms.
The underlying protocol of both mechanisms is as follows:
<ul>
<li> Each RPC is an HTTP POST transaction.
<li> The request and reply messages are strings of the form
".html_text("
param1=val1&param2=val2&...&paramn=valn
")."
there param1 ... paramN are the input or output parameter names,
and val1 ... valn are the values.
All values are URL-encoded using the PHP urlencode() function.
</ul>

<h2>Account creation RPCs</h2>
<p>
The RPC functions are as follows:

<h3>Create account</h3>
";

list_start();
list_item("URL", "project_url/am_create.php");
list_item(
	"input", "email address
		<br>nonce ID (crypto-random string)"
);
list_item(
	"output", "status (integer; 0=success)"
);
list_item(
	"action",
		"The server creates a tentative account.
		The server sends email to the given address, of the form:
        <pre>
Someone (hopefully you) joined [project name] with this email address.
To confirm your participation in [project name] please visit the following URL:
    xxx

If you do not want to participate in [project name], just ignore this message.
        </pre>
		When the participant visits xxx, the account is confirmed.
");
list_end();

echo "
<h3>Query account</h3>
";

list_start();
list_item("URL", "project_url/am_query.php");
list_item("input",
    "nonce ID"
);
list_item("output",
    "status (integer; 0 means account has been confirmed)
    <br>account key (string)
    "
);
list_item("action",
    "If the account has been confirmed, return status=0 and the account key."
);
list_end();

echo "
<h3>Get account info</h3>
";

list_start();
list_item("URL", "project_url/am_get_info.php");
list_item("input", "account key");
list_item("output",
	"status (integer)
    <br>name (string)
    <br>country (string)
    <br>postal_code (string)
    <br>global_prefs (XML string)
    <br>project_prefs (XML string)
    <br>url (string)
    <br>send_email (integer)
    <br>show_hosts (integer)
    "
);
list_end();
echo "
<h3>Set account info</h3>
";
list_start();
list_item("URL", "project_url/am_set_info.php");
list_item("input", "account key
    <br>name (string)
    <br>country (string)
    <br>postal_code (string)
    <br>global_prefs (XML string)
    <br>project_prefs (XML string)
    <br>url (string)
    <br>send_email (integer)
    <br>show_hosts (integer)
    "
);
list_item("output", "status (integer)");
list_end();

echo "
<h2>Account manager RPCs</h2>

";

list_start();
list_item("URL", "Given in the file account_manager_url.xml,
    included in the installer"
);
list_item("input", "name (string)
    <br>password (string)"
);
list_item("output",
    "url1, key1, ..., urln, keyn: list of accounts.
    Each account consists of a (URL, account key) pair."
);
list_end();

page_tail();
?>
