<?
require_once("docutil.php");
page_head("update_versions");
echo "

<h3>Releasing application versions</h3>

<code>update_versions</code>
releases new application versions.
It makes the needed database entries and copies files
to the download directory,
from where they will be distributed to participants.
<p>
To use:
<ul>
<li> Create an 'apps directory' under the project directory.
Add an ", htmlspecialchars("<app_dir>"),
" element to config.xml giving the path of the apps directory.
<li> Create a subdirectory for each application,
with the same name as the application.
Put new application versions here.
</ul>
<p>
<code>update_versions</code>
scans these directories for new core client and application versions,
copies them to the download directory,
and updates the database as appropriate.

<p>

File names must be of the form <code>NAME_VERSION_PLATFORM[.ext]</code>, e.g.:
<pre>
boinc_3.17_i686-pc-linux-gnu.gz
astropulse_7.17_windows_intelx86.exe
</pre>
The prefix name and extensions .gz, .exe, .sit are ignored.
Platform strings must match the names of platforms in the database.

<p>
TO DO: check for code signature files.

<h3>Multiple-file application versions</h3>

Application versions can consist of multiple files.
Follow the above procedure, but create a directory
with the given name, and put the files in that directory.
The executable file with the lexicographically first name
will be treated as the main program.


<h3>Releasing core client versions</h3>

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

";
   page_tail();
?>

