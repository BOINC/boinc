<?php
require_once("docutil.php");
page_head("Retrieving file lists");
echo "
To instruct a host to send a list of all persistent files,
use the function
<pre>
request_file_list(int host_id)
</pre>
or the command line program
<pre>
request_file_list [ -host_id X ]
</pre>
If -host_id is absent, get file lists for all hosts.

<p>
A message is created for the specific host (or all hosts) and added to the 
msg_to_host table in the database.
The upload message included in the next RPC reply to the host.
<p>
???? how do you tell it where to upload the list???
";
page_tail();
?>
