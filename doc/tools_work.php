<?php
require_once("docutil.php");
page_head("Generating work");
echo "

As described earlier, a <a href=work.php>workunit</a>
represents the inputs to a computation.
The steps in creating a workunit are:
<ul>

<li> Write XML 'template files' that describe the workunit
and its corresponding results.
Generally the same templates will be used for
a large number work of workunits.

<li> Create the workunit's input file(s)
and place them in the download directory.

<li> Call a BOINC function that creates a
database record for the workunit.

</ul>
Once this is done, BOINC takes over:
it creates one or more results for the workunit,
distributes them to client hosts,
collects the output files,
finds a canonical result,
assimilates the canonical result,
and deletes files.

<p>
During the testing phase of a project,
you can use the <a href=busy_work.php>make_work</a> daemon
to replicate a given workunit as needed to maintain
a constant supply of work.
This is useful while testing and debugging the application.


<h2>Workunit and result template files</h2>
<p>
A workunit template file has the form
<pre>",htmlspecialchars("
<file_info>
    <number>0</number>
    [ <sticky/>, other attributes]
</file_info>
[ ... ]
<workunit>
    <file_ref>
        <file_number>0</file_number>
        <open_name>NAME</open_name>
    </file_ref>
    [ ... ]
    [ <command_line>-flags xyz</command_line> ]
    [ <rsc_fpops_est>x</rsc_fpops_est> ]
    [ <rsc_fpops_bound>x</rsc_fpops_bound> ]
    [ <rsc_memory_bound>x</rsc_memory_bounds> ]
    [ <rsc_disk_bound>x</rsc_disk_bounds> ]
    [ <delay_bound>x</delay_bound> ]
    [ <min_quorum>x</min_quorum> ]
    [ <target_nresults>x</target_nresults> ]
    [ <max_error_results>x</max_error_results> ]
    [ <max_total_results>x</max_total_results> ]
    [ <max_success_results>x</max_success_results> ]
</workunit>
"), "
</pre>
The components are: 
";
list_start();
list_item(htmlspecialchars("<file_info>, <file_ref>"),
"Each pair describes an <a href=files.php>input file</a>");
list_item(htmlspecialchars("<command_line>"),
"The command-line arguments to be passed to the main program.");
list_item("Other elements",
    "<a href=work.php>Work unit attributes</a>"
);
list_end();
echo"
When a workunit is created, the template file is processed as follows:
<ul>
<li>
Within a &lt;file_info> element,
&lt;number>x&lt;/number> identifies the order of the file.
It is replaced with elements giving
the filename, download URL, MD5 checksum, and size.
<li>
Within a &lt;file_ref> element,
&lt;file_number>x&lt;/file_number> is replaced with an element
giving the filename.
</ul>

<p>
A result template file has the form
<pre>", htmlspecialchars("
<file_info>
    <name><OUTFILE_0/></name>
    <generated_locally/>
    <upload_when_present/>
    <max_nbytes>32768</max_nbytes>
    <url><UPLOAD_URL/></url>
</file_info>
<result>
    <file_ref>
        <file_name><OUTFILE_0/></file_name>
        <open_name>result.sah</open_name>
    </file_ref>
</result>
"), "</pre>
<p>
The result file template is macro-substituted as follows:
<ul>
<li>
&lt;OUTFILE_n> is replaced with a string of the form
'wuname_resultnum_n' where wuname is the workunit name and resultnum is
the ordinal number of the result (0, 1, ...).
<li>
&lt;UPLOAD_URL/> is replaced with the upload URL.
</ul>
<p>

<h2>Moving input files to the download directory</h2>

If you're a flat download directory, just put input files in that directory.
If you're using <a href=hier_dir.php>hierarchical upload/download directories</a>,
you must put each input file in the appropriate directory;
the directory is determined by the file's name.
To find this directory, call the C++ function
<pre>
dir_hier_path(
    const char* filename,
    const char* root,       // root of download directory
    int fanout,             // from config.xml
    bool new_hash,          // use true here
    char* result            // path of file in hierarchy
);
</pre>
If you're using scripts, you can invoke the program
<pre>
dir_hier_path filename
</pre>
It prints the full pathname.
Run this in the project's root directory.
For example:
<pre>
cp test_workunits/12ja04aa `bin/dir_hier_path 12ja04aa`
</pre>
copies an input file from the test_workunits directory
to the download directory hierarchy.


<h2>Creating workunit records</h2>
<p>
Workunits can be created using either a script
(using the <code>create_work</code> program)
or a program (using the <code>create_work()</code> function).
The input files must already be in the download directory.
<p>
The utility program is
<pre>
create_work
    -appname name                       // application name
    -wu_name name                       // workunit name
    -wu_template filename               // WU template filename
                                        // relative to project root
    -result_template filename           // result template filename
                                        // relative to project root
    [ -batch n ]
    [ -priority n ]

    // The following are normally supplied in the WU template:
    [ -rsc_fpops_est x ]
    [ -rsc_fpops_bound x ]
    [ -rsc_memory_bound x ]
    [ -rsc_disk_bound x ]
    [ -delay_bound x ]
    [ -min_quorum x ]
    [ -target_nresults x ]
    [ -max_error_results x ]
    [ -max_total_results x ]
    [ -max_success_results x ]

    infile_1 ... infile_m           // input files
</pre>
The workunit parameters are documented <a href=work.php>here</a>.
The program must be run in the project root directory.

<p>
The C++ library (crypt.C, backend_lib.C,h) provides the functions:
<pre>
int read_key_file(char* path, R_RSA_PRIVATE_KEY& key);

int create_work(
    DB_WORKUNIT&,
    const char* wu_template,                  // contents, not path
    const char* result_template_filename,     // relative to project root
    const char* result_template_filepath,    // absolute,
        // or relative to current dir
    const char* infile_dir,                   // where input files are
    const char** infiles,                     // array of input file names
    int ninfiles
    R_RSA_PRIVATE_KEY& key,             // upload authentication key
    SCHED_CONFIG&
);
</pre>
<p>
<b>read_key_file()</b> reads a private key from a file.
Use this to read the file upload authentication key.
<p>
<b>create_work()</b>
creates a workunit and one or more results.
The arguments are similar to those of the utility program;
some of the information is passed in the WORKUNIT structure,
namely the following fields:
<pre>
name
appid
batch
rsc_fpops
rsc_iops
rsc_memory
rsc_disk
delay_bound
</pre>
All other fields should be zeroed.

";
    
page_tail();
?>
