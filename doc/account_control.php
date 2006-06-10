<?php
require_once("docutil.php");
page_head("Controlling account creation");
echo "
Under normal circumstances BOINC projects are open
for participation by anybody who wants to contribute their computer
to the project.
There may be times, however, when a project needs
to limit the creation of new accounts.
BOINC offers two alternatives.

<h2>Disabling account creation</h2>

To disable all account creation,
edit the project configuration file config.xml and add to it the element:
<pre>
&lt;disable_account_creation>1&lt;/disable_account_creation>
</pre>
This disables account creation via any mechanism
(the client, the web, or account managers).
You can momentarily remove this element while you create accounts.

<h2>Restricting account creation via 'invitation codes'</h2>
<p>
It is also possible to restrict account creation to only those
who present a secret 'invitation code'.
In this case an account can only be created via the web pages,
not via the client or an account manager.

<p>
To use this mechanism you need to add to the file html/project/project.inc
a definition for a PHP pre-processor symbol INVITE_CODES
containing the allowed invitation codes.
A simple example is:
<pre>
define('INVITE_CODES', '/xyzzy/');
</pre>
This allows account creation only if the user enters the invitation code
'xyzzy' (without any quotes).
The pattern in INVITE_CODES is compared to the user's input as a
<a href=http://us2.php.net/manual/en/reference.pcre.pattern.syntax.php>Perl-Compatible Regular Expression (PCRE)</a>,
so don't forget the enclosing slashes.
A more complicated example is:
<pre>
define('INVITE_CODES', '/yohoho|blunderbuss|!grog4U/');
</pre>
In a PCRE vertical bars separate alternatives,
so this pattern just allows someone to create an account
if they enter any of the words 'yohoho', 'blunderbuss', or '!grog4U'.
More complex pattern matching is possible, though not required.

<p>
The security of this mechanism depends on how
you distribute the invitation codes.
If you write the code on the whiteboard in your lab then only
someone with access to that room can use it.
If you send it out to a mailing list then only members of that list can use it
(until someone shares it with someone else who is not on the list).
The goal here is not strict security so much as a way for a new project
to limit account creation to a restricted set of users
while the project is getting started. 
";
page_tail();
?>
