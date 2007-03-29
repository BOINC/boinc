<?php
require_once("docutil.php");
page_head("Data server protocol");
echo "

Core client communicate with data servers using HTTP.
A data server is typically implemented using
a web server together with a BOINC-supplied CGI program,
<b>file_upload_handler</b>.

<h3>Download</h3>
<p>
File download is done by a GET request
to a URL from the FILE_INFO element.
The file offset, if any, is given in <b>Range:</b> attribute
of the HTTP header.

<h3>Upload</h3>
<p>
File upload is done using POST operations to a CGI program.
A security mechanism prevents unauthorized
upload of large amounts of data to the server.
Two RPC operations are used.
<p>
<b>1) Get file size</b>
<p>
The request message has the form:
<pre> ", htmlspecialchars("
<data_server_request>
    <core_client_major_version>1</core_client_major_version>
    <core_client_minor_version>1</core_client_minor_version>
    <core_client_release>1</core_client_release>
    <get_file_size>filename</get_file_size>
</data_server_request>
"), "</pre>

<p>
The reply message has the form:
<pre> ", htmlspecialchars("
<data_server_reply>
    <status>x</status>
    [ <message>text<message>
    | <file_size>nbytes</file_size> ]
</data_server_reply>
"), "</pre>
Status is";
list_start();
list_item("0", "Success.  Nbytes is 0 if the file doesn't exist.");
list_item("1", "Transient error.
The client should try another data server, or try this one later.
");
list_item("-1", "Permanent error.  The client should give up on the result.");
list_end();
echo "
In the error cases, the &lt;file_size> element is omitted
and the &lt;message> element gives an explanation.
<p>
<b>2) Upload file</b>
<p>
Request message format:
<pre> ", htmlspecialchars("
<data_server_request>
<core_client_major_version>1</core_client_major_version>
<core_client_minor_version>1</core_client_minor_version>
    <core_client_release>1</core_client_release>
<file_upload>
<file_info>
   ...
<xml_signature>
   ...
</xml_signature>
</file_info>
<nbytes>x</nbytes>
<md5_cksum>x</md5_cksum>
<offset>x</offset>
<data>
... (nbytes bytes of data; may include non-ASCII data)
(no closing tags)
"), "</pre>
<p>
The &lt;file_info> element is the exact text sent from the
scheduling server to the client.
It includes a signature based on the project's file upload
authentication key pair.
&lt;nbytes> is the size of the file.
&lt;md5_cksum> is MD5 of the entire file.
&lt;offset> is the offset within the file.
<p>
Reply message format:
<pre> ", htmlspecialchars("
<data_server_reply>
    <status>x</status>
    <message>text</message>
</data_server_reply>
"), "</pre>
Status is ";

list_start();
list_item("0", "success");
list_item("1", "transient error;
The client should try another data server, or try this one later.
");
list_item("-1", "Permanent error.
The client should give up on the result.
");
list_end();
echo "
In the error cases, the &lt;message> element gives an explanation.
<p>
TODO: if there's an error in the header
(bad signature, or file is too large)
the client should learn this without actually uploading the file.
";
page_tail();
?>
