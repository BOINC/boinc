<?
require_once("docutil.php");
page_head("Trickle messages");
echo "
<h2>Trickle messages</h2>
<p>
<b>Trickle messages</b>
let applications communicate with the server
during the execution of a workunit.
They are intended for applications that have
long work units (multiple days).
Trickle messages may go in either direction:
'trickle up' messages go from application to server,
'trickle down' messages go from server to application.
Typical uses of this mechanism:
<ul>
<li>
The application sends a trickle-up message containing
its current CPU usage,
so that users can be granted incremental credit
(rather than waiting until the end of the work unit).

<li>
The application sends a trickle-up message
containing a summary of the computational state,
so that server logic can decide if the computation should be aborted.

<li>
The server sends a trickle-down message
telling the application to abort.

<li>
The server sends a trickle-down message
containing the user's current total credit.

</ul>

<p>
Trickle messages are asynchronous and reliable.
Trickle messages are conveyed in scheduler RPC messages,
so they may not be delivered immediately after being generated.


<p>
To handle trickle-down messages a project must include the line
<pre>
&lt;trickle_down/>
</pre>
in the <a href=configuration.php>configuration</a> (config.xml) file.


<h3>API (client)</h3>

<pre>
<code>int boinc_send_trickle_up(char*)</code>
</pre>
sends a trickle message.
Returns zero if success.
<p>
<pre>
<code>bool boinc_receive_trickle_down(char* buf, int len)</code>
</pre>
receives a trickle message.
Returns true if there was a message.
Messages are delivered in order.

<h3>API (server)</h3>

The server C library API:
",html_text("
int get_trickle_up(
    int applicationid,
    int& resultid,
    int& messageid,
    char* buf,
    int len
);
int mark_trickle_up_processed(int messageid);

int send_trickle_down(
    int resultid,
    char* buf
);
"),"
get_trickle_up() gets an unprocessed trickle-up message
for the given application,
returning the result and message IDs as well as the message itself.
mark_trickle_up_processed() marks a trickle-up message as processed.
send_trickle_down() sends a trickle-down message to the
client handling the given result.
<p>
These functions are also available as scriptable command-line programs.

<h3>Implementation</h3>
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
it scans the project directory for trickle-up files
and includes them in the request.
On successful RPC completion it deletes the files.

<p>
On the server,
messages are stored in database tables 'trickle_up' and 'trickle_down'.
<p>
boinc_receive_trickle_down() creates a trickle_down record.
The scheduler RPC handler checks this table for unsent
messages for the given host.
<p>
The scheduling server extracts trickle messages from
the request message and inserts them in trickle_up.


";

page_tail();
?>
