<?php
require_once("docutil.php");
page_head("Participating in multiple projects");
echo "
<p>

You can join a second and subsequent projects as follows.
<ol>
<li> Visit the new project's web site and create an account.
Note: if you wish, you can use different email and name
than those of your first account.
<li> Receive an email containing an account ID (a long random string).
<li> Run the BOINC client, and select the <b>Attach to Project</b> command.
Enter the project's URL and your account ID.

</ol>

<h3>Where to edit your preferences</h3> 
<p>
Your preferences are stored on BOINC servers.
When your hosts communicate with a server they get the latest preferences,
and they pass along these preferences to other servers.
Thus, when you change your preferences on one project's web site,
these changes will quickly spread to all your hosts,
and to the web sites of all the other projects in which you participate.

<p>
If you change your preferences first at one project
and then at another, the second changes will overwrite the first.
To avoid this, do all your edits at one project.

<p>
Some projects may provide a web interface
for editing their project-specific preferences.
In this case it may be necessary to edit preferences at different sites.
To avoid overwriting edits,
wait until previous edits have propagated to a site
before editing preferences there.
";
page_tail();
?>
