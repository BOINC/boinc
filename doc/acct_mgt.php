<?php
require_once("docutil.php");

page_head("External account management <br>(work in progress)");

echo "
<p>
Currently BOINC provides only a web-based interface
for creating and managing accounts.
Participants must locate the web sites of BOINC projects,
read them, decide which to join,
and fill out a separate registration form at each site.
This may deter some potential participants.

<p>
We wish to enable new ways for people to find and join BOINC projects.
For example, one could have a <b>sharing control panel</b> that shows:
<ul>
<li> a list of BOINC projects, with short descriptions and 'join' checkboxes;
<li> a simple form for the user's email address and basic preferences.
</ul>
The user can join projects simply by checking boxes, clicking OK,
and responding to account-verification emails.

<p>
The sharing control panel is an example of what we will call
'account management applications'.

<p>
This document describes a mechanism that allow account
management applications to interact with BOINC projects.

<h2>RPCs for account management</h2>
<p>
We propose having BOINC projects provide
an XML-RPC interface for account management.
RPCs will use HTTP on port 80,
so it will be easy to implement the client side in any language
(C++, Visual Basic, etc.),
and the mechanism will work through firewalls that allow outgoing web requests.

<p>
The proposed RPC functions are as follows:

<h3>Create tentative account</h3>
";
list_start();
list_item(
	"input", "email address
		<br>host name
		<br>client nonce ID (crypto-random)"
);
list_item(
	"output", "tentative account ID"
);
list_item(
	"action",
		"The server creates a 'tentative account' database record.
		The server sends email to the given address, of the form:
        <pre>
Someone (hopefully you) joined [project name] with this email address.
To confirm your participation in [project name] please visit the following URL:
    xxx

If you do not want to participate in [project name], just ignore this message.
        </pre>
		When the user visits xxx, they see a release form and OK button.
		The OK button validates the tentative account,
        creating a new user record if needed.
");
list_end();

echo "
<h3>Query account status</h3>
";
list_start();
list_item("input",
    "tentative account ID
    <br>client nonce ID
    "
);
list_item("output",
    "bool validated
    <br>account key
    "
);
list_item("action",
    "If the account has been validated, return true and the account key.
    The account management application
    can then do <a href=gui_rpc.php>BOINC GUI RPC</a> to the BOINC core client
    to attach to the project."
);
list_end();

echo "
<p>
Possible additions:
RPCs to get and set preferences.

";

page_tail();
?>
