<?php
require_once("docutil.php");
page_head("Applications and versions");
echo "
<p>
An <b>application</b> represents a particular distributed computation;
it consists of a program (perhaps with versions for different platforms)
and a set of workunits and results.
A project can operate many applications. 
Applications are maintained in the <b>application</b> table in the BOINC DB,
and can be created using the <a href=tools_other.php>add</a> utility. 

<p>
An application program may go through a sequence of
<a href=boinc_version.php>versions</a>. 
A particular version, compiled for a particular platform, is
called an <b>application version</b>. 
An application version can consist of multiple files: for example, a
controller script, pre- and post-processing programs, and a primary executable.

<p>
Each application version has an integer <b>version number</b>.
Projects can assign version numbers however they like; for example,
version 304 might represent major version 3 and minor version 4. 
Version numbers should be used consistently across platforms;
Windows version 304 should be computationally identical to
Mac version 304. 

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
<pre>
&lt;file_info&gt; ... 
&lt;/file_info&gt;
[ ... 
]
&lt;app_version&gt;
    &lt;app_name&gt;foobar&lt;/app_name&gt;
    &lt;version_num&gt;4&lt;/version_num&gt;
    &lt;file_ref&gt;
        &lt;file_name&gt;program_1&lt;/file_name&gt;
        &lt;main_program/&gt;
    &lt;/file_ref&gt;
    &lt;file_ref&gt;
        &lt;file_name&gt;library_12&lt;/file_name&gt;
    &lt;/file_ref&gt;
&lt;/app_version&gt;
</pre>
Application versions can be created using the
<a href=tools_other.php>add</a> utility program.
";
page_tail();
?>
