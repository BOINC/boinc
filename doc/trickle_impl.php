<?php
require_once("docutil.php");
page_head("Trickle message implementation");
echo "
<p>
On the client,
<code>boinc_send_trickle_up()</code>
creates a file 'trickle' in the slot directory
and signals the core client via shared memory.
When the core client gets this signal,
or when the application exits,
it moves the file from 'slot/trickle'
to 'project/trickle_resultid_time'.
<p>
When the core client sends an RPC to a server,
it scans the project directory for these trickle-up files
and includes them in the request.
On successful RPC completion it deletes the files.

<p>
On the server,
messages are stored in database tables 'trickle_up' and 'trickle_down'.
The scheduling server extracts trickle messages from
the request message and inserts them in the trickle_up table.
If the 'trickle_down' flag in the configuration is set,
it scans the database for trickle-down messages for this host
and includes them in the reply message,
clearing the 'handled' flag in the DB record.

<p>
The client parses trickle-down messages
in the scheduler reply,
creates files of the form trickle_down_createtime_id
in the slot directory,
and signals the app via shared memory that a message is available.

";
page_tail();
?>
