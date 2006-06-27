<?php
require_once("docutil.php");
page_head("Legacy applications");
echo "
A 'legacy application' is one for which an executable is available,
but not the source code.
Therefore it cannot use the BOINC API and runtime system.
<p>
Such applications can be run using BOINC, as follows:
<ul>
<li>
Compile the program 'wrapper' from the
<a href=example.php>boinc_samples</a> tree,
producing (say) 'wrapper_5.10_windows_intelx86.exe'.

<li> Create a file 'job.xml=job_0.xml' (0 is a version number) containing
".html_text("
<job_desc>
    <task>
        <application>worker_5.10_windows_intelx86.exe</application>
        [ <stdin_filename>X</stdin_filename> ]
        [ <stdout_filename>X</stdout_filename> ]
        [ <command_line>X</command_line> ]
    </task>
    [ ... ]
</job_desc>
")."
where 'worker' is the application name,
and 'worker_5.10_windows_intelx86.exe'
is a version of your legacy application.
If the application uses stdin and stdout,
supply the logical filenames that they correspond to
(these must match the names in your workunit and result templates).
<li>
In your apps/worker directory, create
a directory 'wrapper_5.10_windows_intelx86.exe'.
Put the files 'wrapper_5.10_windows_intelx86.exe',
'job.xml=job_0.xml', and 'worker_5.10_windows_intelx86.exe'
in this directory.
<li>
Run <a href=tool_update_versions.php>update_versions</a>
to create an app version.

<li>
Create your workunit and result templates;
add the &lt;copy_file> element to all file references.

</ul>

The program 'wrapper' executes your legacy application,
and acts as a proxy for it
(to report CPU time etc.).
Future versions of wrapper may allow you to
run multiple applications in sequence
(as specified in the job.xml file).

<p>
Notes:
<ul>
<li> This requires version 5.5 or higher of the BOINC core client.
<li> The stdin/stdout feature is not implemented yet
<li> Multiple tasks per job is not implemented yet
<li> TODO: provide a way for projects to supply an animated GIFF
which is shown (with user/team credit text) as screensaver graphics.
</ul>

";

page_tail();
?>
