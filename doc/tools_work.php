<?
require_once("docutil.php");
page_head("Generating work");
echo "
Workunits and results can be created using either a utility program
or a C++ function.
<p>
Workunits and results are described by <b>template files</b>,
with placeholders for their input and output files.

<h3>Workunit template files</h3>
<p>
A WU template file has the form
<pre>",htmlspecialchars("
<file_info>
    <number>0</number>
    [ ... ]
</file_info>
[ ... ]
<workunit>
    <file_ref>
        <number>0</number>
        <open_name>NAME</open_name>
    </file_ref>
    [ ... ]
    [ <command_line>-flags xyz</command_line> ]
    [ <env_vars>name=val&name=val</env_vars> ]
</workunit>
"), "
</pre>
The components are: 
";
list_start();
list_item(htmlspecialchars("<file_info>, <file_ref>"),
"Each pair describes an input file");
list_item(htmlspecialchars("<command_line>"),
"The command-line arguments to be passed to the main program.");
list_item(htmlspecialchars("<env_vars>"),
"A list of environment variables in the form
name=value&name=value&name=value.");
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
&lt;file_number>x&lt;/file_number> is replaced with the filename.
</ul>
<h3>Result template files</h3>
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
&lt;UPLOAD_URL> is replaced with the upload URL.
</ul>
<p>
<a name=cmdline>
<h3>Command-line interface</h3>
<p>
The utility program is
<pre>
create_work
    -appname name                      // application name
    -wu_name name                      // workunit name
    -wu_template filename              // WU template filename
    -result_template filename          // result template filename
    [ -db_name x ]                     // database name
    [ -db_passwd x ]                   // database password
    [ -db_host x ]                     // database host
    [ -db_user x ]                     // database user name
    [ -upload_url x ]                  // URL for output file upload
    [ -download_url x ]                // base URL for input file download
    [ -download_dir x ]                // where to move input files
    [ -rsc_fpops_est x ]
    [ -rsc_fpops_bound x ]
    [ -rsc_memory_bound x ]
    [ -rsc_disk_bound x ]
    [ -keyfile x ]                     // path of upload private key
    [ -delay_bound x ]
    [ -min_quorum x ]
    [ -target_nresults x ]
    [ -max_error_results x ]
    [ -max_total_results x ]
    [ -max_success_results x ]
    infile_1 ... infile_m           // input files
</pre>
The workunit parameters are documented <a href=work.php>here</a>.
Defaults for many of the optional arguments
are taken from the <b>config.xml</b> file,
if it's in the current directory.

<h3>C++ function interface</h3>
<p>
The C++ library (backend_lib.C,h) provides the functions:
<pre>
int read_key_file(char* path, R_RSA_PRIVATE_KEY& key);

int create_work(
    WORKUNIT&,
    char* wu_template,                  // contents, not path
    char* result_template_filename,     // path
    int redundancy,
    char* infile_dir,                   // where input files are
    char** infiles,                     // array of input file names
    int ninfiles
    R_RSA_PRIVATE_KEY& key,             // upload authentication key
    char* upload_url,
    char* download_url
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

<a name=make_work>
<h3>Make_work</h3>
<p>
The program
<pre>
make_work -wu_name name -cushion N
</pre>
can be used to create an endless supply of work.
It will create copies of the given work unit
as needed to maintain a supply of at least N unsent results.
";
    
page_tail();
?>
