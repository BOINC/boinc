<?php
require_once("docutil.php");
page_head("Deleting files");
echo "
To delete a file from a host, use the function
<pre>
delete_file(int host_id, const char* file_name)
</pre>
or the command line program (run from the project root dir)
<pre>
delete_file -host_id X -file_name Y
</pre>
<p>
delete_file() creates a message for the specific host and adds it to 
the msg_to_host table.
This message instructs the client to delete the file.
The message is included in the next scheduler reply to the host.
The message XML has the form ", html_text("
<delete_file_info>file_name</delete_file_info>
"),"
";
page_tail();
?>
