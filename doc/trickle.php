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

To handle trickle messages,
use the daemon process 'trickle_handler'.
You must supply the following function:
",html_text("
int handle_trickle(TRICKLE_UP&);
"),"
You may send trickle-down messages, from this function or elsewhere,
as follows:
",html_text("
DB_TRICKLE_DOWN tdown;
// ... populate the tdown object
tdown.insert();
"),"

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
