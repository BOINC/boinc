<?php
require_once("docutil.php");
page_head("Core client: file structure");
echo "
<p>
The core client runs in a <b>BOINC home directory</b>.
It creates and uses the following files and directories within the
home directory: 
<ul>
<li>
<b>prefs.xml</b>:
The user's general preferences; see below.
<li>
<b>client_state.xml</b> describes of the files,
application, workunits and results present on this client.
<li>
<b>Account files</b> describes the participant's account
in each subscribed project,
including the resource share and project-specific preferences.
It contains no host-specific information.
The name of the account file is <b>account_PROJECT.xml</b>,
where PROJECT is an encoded version of the project's master URL.
<li>
A directory <b>projects</b>,
which contains a <b>project directory</b> for each subscribed project.
The name of a project directory is an encoded
version of the project's master URL.
The project directory contains all files (inputs,
outputs, executables) related to the project.
<li>
<b>slots</b>: this directory contains one subdirectory for each
result in progress.
The subdirectories are named 0, 1, ... N-1.
</ul>
<p>
Each result executes in particular slot directory.
The core client creates 'soft link' files in the slot directory,
linking to the corresponding files in the project directory.

<h3>Format of account files</h3>
The format of an account file is as follows:
<pre>
";
echo htmlspecialchars("
<account>
    <master_url>http://www.myproject.com/</master_url>
    <authenticator>3f7b90793a0175ad0bda68684e8bd136</authenticator>
    [ <project_name>...</project_name> ]
    [ <tentative/> ]
    <project_preferences>
    <resource_share>1</resource_share>
    <project_specific>
        ...
    </project_specific>
    [ <venue>...</venue> ]
    </project_preferences>
</account>
");
echo "
</pre>
The &lt;project_preferences&gt; field is
the 'project_prefs' field of the user database record.

<h3>Format of prefs.xml</h3>
<p>
This format of prefs.xml is as follows:

<pre>
&lt;preferences>
    &lt;prefs_mod_time>1030128387&lt;/prefs_mod_time>
    &lt;from_project>http://www.myproject.com/&lt;/from_project>
    &lt;from_scheduler>http://server3.myproject.com/cgi-bin/scheduler_cgi&lt;/from_scheduler>
    &lt;mod_time>2&lt;/mod_time>
    &lt;high_water_days>2&lt;/high_water_days>
    &lt;low_water_days>1&lt;/low_water_days>
&lt;/preferences>

XXX this is not complete
</pre>
";
page_tail();
?>
