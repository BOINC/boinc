<?
require_once("docutil.php");
page_head("Trickle messages");
echo "
<h2>Trickle messages</h2>
<p>
<b>Trickle messages</b>
let applications communicate with the server
during the execution of a workunit.
That are intended for applications that have
long work units (multiple days).
Trickle messages may be in either direction:
'trickle up' messages go from application to server,
'trickle down' messages go from server to application.
Typical uses of this mechanism:
<ul>
<li>
The application reports its current CPU usage,
so that users can be granted incremental credit
(rather than waiting until the end of the work unit).

<li>
The application reports a summary of the computational state,
so that server logic can decide if the computation should be aborted.
</ul>

<p>
Trickle messages are asynchronous and reliable.
A trickle message may not be delivered immediately after it is sent.
If there is a message waiting to be sent,
the BOINC client may contact the scheduling server,
but this may take a while if the client is offline.


<p>
To handle trickle-down messages you must include the line
<pre>
&lt;trickle_down/>
</pre>
in the configuration (config.xml) file.


<h3>API (client)</h3>

<pre>
<code>int boinc_send_trickle(char*)</code>
</pre>
sends a trickle message.
Returns zero if success.
<p>
<pre>
<code>bool boinc_receive_trickle(char* buf, int len)</code>
</pre>
receives a trickle message.
Returns true if there was a message.
Messages are delivered in order.

<h3>API (server)</h3>

The server C library API:
<pre>
<code>
int get_trickle_message(
    int applicationid,
    int& resultid,
    int& messagid,
    char* buf,
    int len
);

int mark_trickle_processed(int messagid);

int send_trickle_message(
    int resultid,
    char* buf
);
</code>
</pre>
get_trickle_message() gets an unprocessed trickle message
for the given application,
returning the result and message IDs as well as the message itself.
mark_trickle_processed() flags a trickle message as processed.
send_trickle_message() sends a trickle message to the
client handling the given result.
<p>
These functions are also available as scriptable command-line programs.

<h3>Implementation</h3>
<p>
On the client,
<code>boinc_send_trickle()</code>
creates a file 'trickle' in the slot directory
and signals the core client via shared memory.
When the core client gets this signal,
or when the application exits,
it moves the file from 'slot/trickle'
to 'project/trickle_resultid_time'.
<p>
When the core client sends an RPC to a server,
it scans the project directory for trickle files
and includes them in the request.
On successful RPC completion it deletes the trickle files.

<p>
boinc_receive_trickle() sets a flag in the result record;
this flag is conveyed to the scheduling server.
<p>
The server database has two tables,
trickle_up and trickle_down.
The scheduling server extracts trickle messages from
the request message and inserts them in trickle_up.
If the above flag is set for a given result,
it queries the trickle_down table for that result
and appends any messages to the reply.


";

page_tail();
?>
