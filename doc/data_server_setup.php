<?php
require_once("docutil.php");
page_head("Setting up a data server");
echo "

<p>
The BOINC data server is implemented using Apache
or a similar web server.
It is used both to upload and to download files.
File download is handled by the web server, using GET operations.
File upload is done by a CGI program, <b>file_upload_handler</b>.
The host need not have access to the BOINC database.
<p>
You must copy the file upload authentication key
to each data server.

<p>
The file upload handler uses a configuration of
the same format as used by the
<a href=sched_server_setup.php>scheduling server</a>.

<h3>Web server configuration</h3>
<p>
You must edit your web server configuration file
to allow access to the download directory
and to the file-upload CGI program.
For example, the addition to Apache's httpd.conf might be:
<pre>
Alias /barry/ \"/users/barry/\"

&lt;Directory \"/users/barry/\">
    Options Indexes FollowSymlinks MultiViews
    AllowOverride None
    Order allow,deny
    Alias from all
&lt;/Directory>

ScriptAlias /boinc-cgi/ \"/users/barry/cgi/\"

&lt;Directory \"/users/barry/cgi/\">
    AllowOverride None
    Options None
    Order allow,deny
    Allow from all
&lt;/Directory>
</pre>
You should also set the default MIME type as follows:
<pre>
DefaultType application/octet-stream
</pre>
";
page_tail();
?>
