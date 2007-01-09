<?php
require_once("docutil.php");
page_head("Applications and versions");
echo "
<p>
An <b>application</b> 
consists of a program (perhaps with versions for different platforms)
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
Each application version has an integer version number.
Version numbers should be used consistently across platforms;
Windows version 304 should be computationally identical to Mac version 304. 

<p>
Each application has a <b>minimum version</b>. 
When a client is sent work for an application,
it is also sent the latest application version for its platform. 
It is sent work only if this version is the minimum or greater. 

<p>
Application versions can be created using
<a href=tool_update_versions.php>update_versions</a>.
Descriptions of application versions are stored in the <b>app_version</b>
table in the BOINC DB.
";
page_tail();
?>
