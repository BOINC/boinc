<? // -*- html -*-
   // $Id$
   require_once("docutil.php");
   page_head("update_versions");
   echo "

<code>update_versions</code>
releases new core client and application versions.
Core client versions will be visible to participants
on the 'Download BOINC' web page.
Application versions will be distributed to participants.
Make sure to test versions before releasing them!
<p>
To use:
<ul>
<li> Create an 'apps directory' under the project directory.
Add an
";
echo htmlspecialchars("<app_dir>");
echo "
element to config.xml giving the path of the apps directory.
<li> Create a subdirectory 'boinc' in the apps directory.
Put new core client versions here.
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


";
   page_tail();
?>

