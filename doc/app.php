<?php
require_once("docutil.php");
page_head("Applications and versions");
echo "
<p>
An <b>application</b> represents a collection of related computation.
It consists of a program (perhaps with versions for different platforms)
and a set of <a href=work.php>workunits</a> and <a href=result.php>results</a>.
A project can operate many applications. 
Applications are maintained in the <b>application</b> table in the BOINC DB,
and can be created using the <a href=tool_xadd.php>xadd</a> utility. 

<p>
An application program may go through a sequence of
<a href=boinc_version.php>versions</a>. 
A particular version, compiled for a particular platform, is
called an <b>application version</b>. 
An application version can consist of multiple files: for example, a
controller script, pre- and post-processing programs, and a primary program.

<p>
Each application version has an integer <b>version number</b>.
This number is of the form 100*major + minor;
for example, 304 represents major version 3 and minor version 4. 
An application version will only run on a core client
having the same major version.
Version numbers should be used consistently across platforms;
Windows version 304 should be computationally identical to Mac version 304. 

<p>
Each application has a <b>minimum version</b>. 
When a client is sent work for an application,
it is also sent the latest application version for its platform. 
It is sent work only if this version is the minimum or greater. 

<p>
Application versions are maintained in the <b>app_version</b> table
in the BOINC DB.
Each entry includes an XML document describing the
files that make up the application version: 
<pre>".htmlspecialchars("
<file_info>
   ... 
</file_info>
[ ... ]
<app_version>
    <app_name>foobar</app_name>
    <version_num>4</version_num>
    <file_ref>
        <file_name>program_1</file_name>
        <main_program/>
    </file_ref>
    <file_ref>
        <file_name>library_12</file_name>
    </file_ref>
</app_version>")."
</pre>
Application versions can be created using
<a href=tool_update_versions.php>update_versions</a>.
";
page_tail();
?>
