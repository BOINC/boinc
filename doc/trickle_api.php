<?php
require_once("docutil.php");
page_head("Trickle message API");

echo "
<h3>API (application)</h3>
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


";
page_tail();
?>
