<?php
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
To handle trickle-down messages, a project must include the line
<pre>
&lt;msg_to_host/>
</pre>
in the <a href=configuration.php>configuration</a> (config.xml) file.



";

page_tail();
?>
