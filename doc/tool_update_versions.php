<?php
require_once("docutil.php");
page_head("Releasing application versions");
echo "

The <code>update_versions</code> script
releases new application versions.
It creates database entries and copies files to the download directory.
<p>
To use:
<ul>
<li>
If it doesn't already exit,
create an directory 'apps' under the project directory,
and add an ", htmlspecialchars("<app_dir>"),
" element to config.xml giving the path of the apps directory.
<li> Create a subdirectory for each application,
with the short name of the application.
Put new application files here (see below).
<code>update_versions</code>
scans these directories for new application versions.
<li> From the project's root directory, run bin/update_versions
</ul>

<h3>Single-file application versions</h3>

File names must be of the form <code>NAME_VERSION_PLATFORM[.ext]</code>, e.g.:
<pre>
boinc_3.17_i686-pc-linux-gnu.gz
astropulse_7.17_windows_intelx86.exe
</pre>
<p>
Notes:
<ul>
<li>
<b>Platform strings must match the names of platforms in the database.</b>
If needed, <a href=tool_xadd.php>add the platform to the DB</a>.
<li>
<b>Applications must have the same major version number
as your BOINC server software</b>.
</ul>

<p>
If a file of the form
<pre>
FILENAME.sig
</pre>
is found, its contents will be used as a digital signature
for the corresponding file.
Recommended code-signing practices are described
<a href=code_signing.php>here</a>.


<p>
If a file of the form
<pre>
FILENAME.file_ref_info
</pre>
is found, its contents will be added to the &lt;file_ref>
element describing the file
(you can use this for attributes like &lt;copy_file>).

<h3>Multiple-file application versions</h3>

Application versions can consist of multiple files,
one of which is the main program.
To create a multiple-file application version,
create a directory with the same name as the main program
(of the form NAME_VERSION_PLATFORM[.ext]).
and put the files in that directory.
<p>
If your application includes executable files other than
the main file, make sure that their protection flags
include the user execute (u+x) bit.


";
   page_tail();
?>

