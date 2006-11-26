<?php
require_once("docutil.php");
page_head("Legacy applications");
echo "
A <b>legacy application</b> is one for which an executable is available,
but not the source code.
Therefore it cannot use the BOINC API and runtime system.
However, such applications can be run using BOINC.
Here's an example:
<ul>
<li>
Compile the program 'worker' from the
<a href=example.php>boinc_samples</a> tree,
producing (say) 'worker_5.10_windows_intelx86.exe'.
This is the legacy app.
If reads from stdin and writes to stdout;
it also opens and reads a file 'in',
and opens and writes a file 'out'.
It takes one command-line argument:
the number of CPU seconds to use.

<li>
Compile the program 'wrapper' from the
<a href=example.php>boinc_samples</a> tree,
producing (say) 'wrapper_5.10_windows_intelx86.exe'.
This program executes your legacy application,
and acts as a proxy for it (to report CPU time etc.).

<li>
<a href=app.php>Create an application</a> named 'worker',
and a corresponding directory 'project/apps/worker'.
In this directory, create
a directory 'wrapper_5.10_windows_intelx86.exe'.
Put the files 'wrapper_5.10_windows_intelx86.exe',
and 'worker_5.10_windows_intelx86.exe' there.

<li> In the same directory,
create a file 'job.xml=job_1.12.xml' (1.12 is a version number) containing
".html_text("
<job_desc>
    <task>
        <application>worker_5.10_windows_intelx86.exe</application>
        <stdin_filename>stdin</stdin_filename>
        <stdout_filename>stdout</stdout_filename>
        <command_line>10</command_line>
    </task>
</job_desc>
")."
This file is read by 'wrapper';
it tells it the name of the legacy program,
what files to connect to its stdin/stdout, and a command-line argument.
<li>
Create a workunit template file
".html_text("
<file_info>
    <number>0</number>
</file_info>
<file_info>
    <number>1</number>
</file_info>
<workunit>
    <file_ref>
        <file_number>0</file_number>
        <open_name>in</open_name>
        <copy_file/>
    </file_ref>
    <file_ref>
        <file_number>1</file_number>
        <open_name>stdin</open_name>
    </file_ref>
    <rsc_fpops_bound>1000000000000</rsc_fpops_bound>
    <rsc_fpops_est>1000000000000</rsc_fpops_est>
</workunit>
")."
and a result template file
".html_text("
<file_info>
    <name><OUTFILE_0/></name>
    <generated_locally/>
    <upload_when_present/>
    <max_nbytes>5000000</max_nbytes>
    <url><UPLOAD_URL/></url>
</file_info>
<file_info>
    <name><OUTFILE_1/></name>
    <generated_locally/>
    <upload_when_present/>
    <max_nbytes>5000000</max_nbytes>
    <url><UPLOAD_URL/></url>
</file_info>
<result>
    <file_ref>
        <file_name><OUTFILE_0/></file_name>
        <open_name>out</open_name>
        <copy_file/>
    </file_ref>
    <file_ref>
        <file_name><OUTFILE_1/></file_name>
        <open_name>stdout</open_name>
    </file_ref>
</result>
")."
Note that the files opened directly by the legacy program
must have the &lt;copy_file&gt; tag.
<li>
Run <a href=tool_update_versions.php>update_versions</a>
to create an app version.

<li>
Run a script like
".html_text("
cp download/input `bin/dir_hier_path input`
cp download/input2 `bin/dir_hier_path input2`

bin/create_work -appname worker -wu_name worker_nodelete -wu_template templates/worker_wu -result_template templates/worker_result input input2
")."
to generate a workunit.

</ul>

<p>
Notes:
<ul>
<li> This requires version 5.5 or higher of the BOINC core client.
<li> Multiple tasks per job is not implemented yet.
Future versions of wrapper may allow you to
run multiple applications in sequence
(as specified in the job.xml file).
<li> TODO: provide a way for projects to supply an animated GIFF
which is shown (with user/team credit text) as screensaver graphics.
</ul>

To understand how all this works:
at the beginning of execution, the file layout is:
";
list_start();
list_heading_array(array("Project directory", "slot directory"));
list_item_array(array(
    "job_1.12.xml
    <br>input
    <br>input2
    <br>worker_5.10_windows_intelx86.exe
    <br>wrapper_5.10_windows_intelx86.exe
    ",
    "
    in (copy of project/input)
    <br>job.xml (link to project/job_1.12.xml)
    <br>stdin (link to project/input2)
    <br>stdout (link to project/worker_nodelete_0)
    <br>worker_5.10_windows_intelx86.exe
     (link to project/worker_5.10_windows_intelx86.exe)
    <br>wrapper_5.10_windows_intelx86.exe
     (link to project/wrapper_5.10_windows_intelx86.exe)
    "
));
list_end();

echo "
The wrapper program executes the worker,
connecting its stdin to project/input2
and its stdout to project/worker_nodelete_0.
The worker program opens 'in' for reading and 'out' for writing.
<p>
When the worker program finishes, the wrapper sees this and exits.
Then the the BOINC core client copies
slot/out to project/worker_nodelete_1.
";

page_tail();
?>
