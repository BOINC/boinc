<?php
require_once("docutil.php");
page_head("Participating in multiple projects");
echo "
<p>

You can join a second and subsequent projects as follows.
<ol>
<li> Visit the second project's web site and create an account.
You don't have to use the same email address,
but if you do so the accounts will be 'coupled'
in terms of preferences and statistics.
<li> Receive an email containing an account ID.
<li> Paste the account ID into the project's web site
to complete the account creation.
<li> Run the BOINC client, and select the <b>Attach to Project</b> command.
Enter the project's URL and your account ID.

</ol>

<h2>General preferences</h2> 
<p>
Each BOINC project has its own database and servers.
Each account has its own copy of your
<a href=prefs.php>general preferences</a>.
When you edit your general preferences on a particular project,
initially it changes only that one account.
However, BOINC will eventually
<ol>
<li> propagate the new preferences to all computers
attached to that account, and
<li> propagate the new preferences to all
accounts that are 'coupled' to the first one
(i.e. that have the same email address,
and have at least one computer attached to both accounts).
</ol>

This propagation is 'piggybacked' onto the
scheduler requests between your computers and project servers.
You can accelerate the propagation by using
the Update command in the BOINC Manager.

<p>
Be careful about editing general preferences at different projects.
If you change your general preferences at project A,
then edit them at project B before the first changes have propagated there,
the second changes will overwrite the first.
To avoid this, pick a 'home project' and do all your edits there.

<h2>Host location</h2>

<p>
Each host attached to a project has its own
record in the database of that project;
this record includes the location (home/work/school) of the host.
BOINC doesn't try to make these agree -
it's possible that a given host has location 'work'
on project A, and location 'school' on project B.
When the BOINC Manager starts up, it shows you
that locations of the host on all the projects
to which it's attached.

<p>
A host's location on a given project determines which
project preferences are used:
i.e., if a host has location 'home' an project A,
and you've defined separate project preferences for 'home',
it will use those preferences.

<p>
The choice of general preferences (if you've defined
separate preferences) is determined by
the host's location on the project from which the
general preferences were propagated,
i.e. from the project where you last edited them.
So if you edit your general preferences on project A,
a host's location on project A is 'work',
and you've defined separate general preferences for 'work',
the host will use those.


";
page_tail();
?>
