<?php
require_once("docutil.php");

page_head("Trickle messages");
echo "
<b>Trickle messages</b>
let applications communicate with the server
during the execution of a workunit.
They are intended for applications that have long workunits (multiple days).

<p>
Trickle messages may from client to server or vice versa.
Messages are XML documents.

<h3>Trickle-up messages</h3>
<p>
<b>Trickle-up</b> messages go from application to server.
They are handled by <b>trickle handler daemons</b> running on the server.
Each message is tagged with a 'variety' (a character string).
Each daemon handles messages of a particular variety.
(This is used, typically, to distinguish different applications.)
Example uses:
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
</ul>

<p>
To create a trickle handler daemon, modify the program
<code>sched/trickle_handler.C</code>, replacing the function
<code>handle_trickle()</code> with your own function.
Add an entry in your <a href=configuration.php>config.xml</a>
to run this program as a daemon.

<h3>Trickle-down messages</h3>
<p>

<b>Trickle-down</b> messages go from server to application.
Each one is addressed to a particular host,
and must include an element <code>&lt;result_name&gt;</code>
identifying the result to which the message is addressed.
If that result is still running on the host, it is delivered to it.
Example uses:

<ul>
<li>
The server sends a message telling the application to abort.

<li>
The server sends a message containing the user's current total credit.

</ul>

<p>
Trickle messages are asynchronous, ordered, and reliable.
Trickle messages are conveyed in scheduler RPC messages,
so they may not be delivered immediately after being generated.

";

page_tail();
?>
