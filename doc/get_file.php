<?php
require_once("docutil.php");
page_head("Retrieving files");
echo "
<p>
A persistent file can be retrieved from a specific host.
This can be done using the function
<pre>
get_file(int host_id, const char* file_name)
</pre>
<p>
or the command line program (run in the project root dir)
<pre>
get_file -host_id X -file_name Y
</pre>
This program must be run in the project's root directory.
<p>
get_file() creates a result with a name of the form:
<pre>
get_FILENAME_HOSTID_TIME
</pre>
<p>
Example: get_test.mpg_34_123456789 is a result representing the 
upload of test.mpg from host number 34 at time 1234567891.
<p>
An upload message is created for the specific host
and added to the msg_to_host table in the database. 
This message instructs the client to upload the file
and acknowledge a successful upload.
The upload message included in the next RPC reply to the host.
The message has the form:
", html_text("
<app>
    <name>FILE_MOVER</name>
</app>
<app_version>
    <app_name>FILE_MOVER</app_name>
    <version_num>BOINC_MAJOR_VERSION</version_num>
</app_version>
<file_info>
    <name>file_name</name>
    <url>upload_dir</url>
    <max_nbytes>1e10</max_nbytes>
    <upload_when_present/>
</file_info>
    RESULT_XML
<workunit>
    <name>result.name</name>
    <app_name>FILE_MOVER</app_name>
</workunit>
"),"
Include ", html_text("<msg_to_host/>"), " in config.xml.
Currently ", html_text("<ignore_upload_certificates/>"), "
needs to be included as there is no
way to send upload certificates with these files. 
";
page_tail();
?>
