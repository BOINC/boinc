<?
require_once("docutil.php");
page_head("Generating work");
echo "
<p>
Workunits and results can be created using either a utility program
or a C++ function.
In either case you must first <a href=key_setup.html>generate a key pair</a>
for file upload authentication.
<hr>
The utility program is
<pre>
create_work
    -appname name                   // application name
    -wu_name name                   // workunit name
    -wu_template filename           // WU template filename
    -result_template filename       // result template filename
    -redundancy n                   // # of results to create
    -db_name x                      // database name
    -db_passwd x                    // database password
    -upload_url x                   // URL for output file upload
    -download_url x                 // base URL for input file download
    -download_dir x                 // where to move input files
    -rsc_fpops x                    // est. # floating-point ops
    -rsc_iops x                     // est. # integer ops
    -rsc_memory x                   // est. RAM working set size, bytes
    -rsc_disk x                     // est. disk space required
    -keyfile x                      // path of upload private key
    -delay_bound x                  // delay bound for result completion
    infile_1 ... infile_m           // input files
</pre>
<p>
The WU template file has the form
<pre>
[ &lt;file_info>...&lt;/file_info> ]
[ ... ]
&lt;workunit>
    [ &lt;command_line>-flags xyz&lt;/command_line> ]
    [ &lt;env_vars>name=val&amp;name=val&lt;/env_vars> ]
    [ &lt;max_processing>...&lt;/max_processing> ]
    [ &lt;max_disk>...&lt;/max_disk> ]
    [ &lt;file_ref>...&lt;/file_ref> ]
    [ ... ]
&lt;/workunit>
</pre>
The components are: 
<table border=1 cellpadding=6>
<tr><td>&lt;command_line></td>
<td>The command-line arguments to be passed to the main program.
</td></tr>
<tr><td>&lt;env_vars></td>
<td>A list of environment variables in the form
name=value&name=value&name=value.
</td></tr>
<tr><td valign=top>&lt;max_processing></td>
<td>Maximum processing
(measured in <a href=credit.html>Cobblestones</a>).
An instance of the computation that exceeds this bound will be aborted.
This mechanism prevents an infinite-loop bug from
indefinitely incapacitating a host.
The default is determined by the client; typically it is 1.
</td></tr>
<tr><td>&lt;max_disk></td>
<td>Maximum disk usage (in bytes).
The default is determined by the client; typically it is 1,000,000.
</td></tr>
<tr><td>&lt;file_ref></td>
<td> describes a <a href=files.html>reference</a> to an input file,
each of which is described by a <b>&lt;file_info></b> element.
</td></tr></table>
<p>
The workunit template file is processed as follows:
<ul>
<li>
Within a &lt;file_info> element,
&lt;number>x&lt;/number> identifies the order of the file.
It is replaced with elements giving
the filename, download URL, MD5 checksum, and size.
file.
<li>
Within a &lt;file_ref> element,
&lt;file_number>x&lt;/file_number> is replaced with the filename.
</ul>
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
<hr>
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
";
page_tail();
?>
