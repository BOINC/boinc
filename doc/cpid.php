<?php
require_once("docutil.php");

page_head("Cross-project identification");

echo "
<p>
Leaderboard sites may show statistics from several BOINC projects,
and may want to show credit for users and/or hosts
summed across all the projects in which they participate.

<h2>Cross-project identification of hosts</h2>

<p>
Each host generates an <b>internal cross-project ID</b>,
which is the MD5 of the concatenation of its
domain name, IP address, free disk space, and a timestamp.
This is reported to the projects that to which the host is attached.
The projects convert it to an <b>external cross-project ID</b>
by hashing it with the owner's email address
(this is intended to prevent spoofing).
The external ID is exported in statistics files.

<h2>Cross-project identification of participants</h2>
<p>
Accounts on different projects are considered equivalent
if they have the same email address
(we have considered other concepts, but they all lead to extreme complexity).

<p>
Projects can't export email addresses in statistics files;
email addresses are private.
It's also not desirable to export hashed email addresses,
because spammers could enumerate feasible email addresses
and compare them with the hashed addresses.

<p>
Instead, BOINC uses the following system:
<ul>
<li>
Each account is assigned an 'internal cross-project identifier' (CPID)
when it's created; it's a long random string.
<li>
When a scheduling server replies to an RPC,
it includes the account's CPID,
its email address, and its creation time.
These are stored in the client state file.
<li>
When the BOINC client makes an RPC request to a scheduling server,
it scans the accounts with the same email address,
finds the one with the oldest creation time,
and sends the CPID stored with that account.
<li>
If the scheduling server receives a CPID different
from the one in its database, it updates the database with the new CPID.
<li>
User elements in the XML download files include
a hash of (email address, CPID);
this 'external' CPID serves as a unique identifier of all
accounts with that email address.
(The last step, hashing with the email address,
prevents people from impersonating other people).
</ul>

This system provides cross-project identification based on email address,
without publicizing any information from which
email addresses could be derived.

";

page_tail();
?>
