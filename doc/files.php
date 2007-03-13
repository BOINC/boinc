<?php
require_once("docutil.php");
page_head("Storage");
echo "
<h3>Files and data servers</h3> 
<p>
The BOINC storage model is based on <b>files</b>.
Examples of files:
<ul>
<li> The inputs and/or outputs of computation;
<li> Components of application: executables, libraries, etc.
<li> Data for its own sake,
e.g. to implement a distributed storage system.
</ul>
<p>
The BOINC core client transfers files to and from project-operated
<b>data servers</b> using HTTP.
<p>
Once a file is created (on a data server or a participant host) it
is <b>immutable</b>.
This means that all replicas of that file are assumed (and required)
to be identical.
If a file is changed, even by a single byte, it becomes a new file,
and must be given a different name than the original.

<a name=file></a>
<h3>File properties</h3>
Files have various properties, including:
<ul>
<li> <b>Name</b>: unique identifier for the file.
Must be nonempty, at most 255 chars, and may not contain '..' or '%'.
<li> <b>Sticky</b>: don't delete file on client (see below).
<li> <b>Report on RPC</b>: include a description of this file
in scheduler requests.
<li> <b>Maximum size</b>: if an output file exceeds its maximum size,
the computation is aborted.
</ul>

File properties are specified in <a href=xml.php>XML elements</a>
that appear, for example, in <a href=tools_work.php>workunit templates</a>.

<a name=file_ref></a>
<h3>File references</h3>
In addition to the properties of a file itself,
there are several properties of the way a particular
application uses a file;
this is called a <b>file reference</b>.
<ul>
<li> <b>open_name</b> (or 'logical name'):
the name by which the program will refer to the file
(BOINC provides a mechanism for mapping logical names
to physical names, so that your applications don't
have to know the physical names of their input and output files).
<li> <b>copy file</b>:  use this if your application
doesn't use BOINC's filename-mapping mechanism.
<li> <b>optional</b>: Use this for output files, to indicate
that the application doesn't always generate the file,
and that its absence should not be treated as an error.
</ul>
File reference properties are specified in
<a href=xml.php#file_ref>XML elements</a>.

<a name=file_ref></a>
<h3>File management</h3>
<p>
BOINC's default behavior is to delete files
when they aren't needed any more.
Specifically:
<ul>
<li> On the client, input files are deleted when no workunit refers to them,
and output files are deleted when no result refers to them.
Application-version files are deleted when they are referenced
only from superceded application versions.
<li> On the client, the 'sticky' flag overrides the above mechanisms
and suppresses the deletion of the file.
The file may deleted by an explicit
<a href=delete_file.php>server request</a>.
The file may also be deleted at any time by the core client
in order to honor limits on disk-space usage.
<li> On the server, the <a href=file_deleter.php>file deleter daemon</a>
deletes input and output files that are no longer needed.
This can be suppressed using the 'no_delete' flag,
or using command-line options to the file deleter.
</ul>

<a name=compression></a>
<h3>Compression of input files</h3>

<p>
Starting with version 5.4, the BOINC client
is able to handle HTTP Content-Encoding types 'deflate' (zlib algorithm)
and 'gzip' (gzip algorithm).
The client decompresses these files 'on the fly' and
stores them on disk in uncompressed form.
<p>
You can use this in two ways:

<ul>
<li>
Use the Apache 2.0 mod_deflate module to automatically
compress files on the fly.
This method will work with all BOINC clients,
but it will do compression only for 5.4+ clients.
Here is a <a href=apache_deflate.txt>cookbook</a>
on how to configure this.

<li>
Compress files and give them a filename suffix such as '.gz'.
The name used in your <code>&lt;file_info&gt;</code>
elements, however, is the original filename without '.gz'.
<p>
Include the following line in httpd.conf:
<pre>
AddEncoding x-gzip .gz
</pre>

This will add the content encoding to the header so that
the client will decompress the file automatically.
This method has the advantage of reducing server disk usage
and server CPU load,
but it will only work with 5.4+ clients.
Use the 'min_core_version' field of the app_version table to enforce this.


You can use this in conjunction because the mod_deflate module
allows you to exempt certain filetypes from on-the-fly compression.
</ul>

<p>
Both methods store files uncompressed on the client.
If you need compression on the client,
you must do it at the application level.
The BOINC source distribution includes
<a href=boinc_zip.txt>a version of the zip library</a>
designed for use by BOINC applications on any platform.

<h3>Compression of output files</h3>
<p>
If you include the <code>&lt;gzip_when_done&gt;</code>
tag in an <a href=xml.php#file>output file description</a>,
the file will be gzip-compressed after it has been generated.
";
page_tail();
?>
