<?php
require_once("docutil.php");
page_head("update_versions");
echo "

<h2>Releasing application versions</h2>

The <code>update_versions</code> script
releases new application versions.
It makes the needed database entries and copies files
to the download directory.
<p>
To use:
<ul>
<li> Create an 'apps directory' under the project directory.
Add an ", htmlspecialchars("<app_dir>"),
" element to config.xml giving the path of the apps directory.
<li> Create a subdirectory for each application,
with the same name as the application.
Put new application versions here.
<code>update_versions</code>
scans these directories for new application versions.
</ul>
<p>

File names must be of the form <code>NAME_VERSION_PLATFORM[.ext]</code>, e.g.:
<pre>
boinc_3.17_i686-pc-linux-gnu.gz
astropulse_7.17_windows_intelx86.exe
</pre>
The prefix name and extensions .gz, .exe, .sig are ignored.
<p>
Important notes:
<ul>
<li> <b>Platform strings must match the names of platforms in the database.</b>
    If needed, <a href=tool_xadd.php>add the platform to the DB</a>.
<li> <b>Your application must have the same major version number
as your BOINC server software</b>.
</ul>

<p>
If a file of the form
<pre>
EXEC_FILENAME.sig
</pre>
is found, its contents will be used as a digital signature
for the corresponding file.
Recommended code-signing practices are described
<a href=code_signing.php>here</a>.


<h3>Minimum core version</h3>
Application versions have a field <code>min_core_version</code> which,
if nonzero, indicates the minimum core client version number
to which the application version should be sent.
Update_versions, by default, sets this to the largest
core client version number in the database.
You can disable this by editing update_versions,
or by manually changing the app_version record.

<h3>Multiple-file application versions</h3>

Application versions can consist of multiple files.
Follow the above procedure, but create a directory
with the given name, and put the files in that directory.
The executable file with the lexicographically first name
will be treated as the main program.


<h2>Releasing core client versions</h2>

The same mechanism is used to release core client versions:
Create a subdirectory 'boinc' in the apps directory,
put new core client installer files there, and run update_versions.
Core client versions will be visible to participants
on your project's 'Download BOINC' web page.

<p>
<b>NOTE</b>: the files distributed in this way are installers,
not executables.
Instructions for creating installers are
<a href=ssl_client_release_instructions.txt>here</a>.
<p>
<b>NOTE</b>: in the interests of consistency,
we recommend that BOINC projects not distribute core client versions,
but rather set the 'Download BOINC' link on their web page
to point to the download page on the main BOINC site.
Use this URL:
http://setiweb.ssl.berkeley.edu/sah/download_boinc.php

";
   page_tail();
?>

