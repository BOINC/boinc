<?
require_once("docutil.php");
page_head("Core client: finite-state machine (FSM) structure");
echo "
<p>
The core client can perform many activities (file transfers,
computations, RPCs to scheduling servers) in parallel.
To manage this parallelism, the core client is structured as a number of
<b>finite-state machines</b> (FSM).
For example, an HTTP transaction is
represented by an FSM whose states might include: 
</p>
<ul>
<li> Waiting for connection establishment. 
<li> Waiting to send request header. 
<li> Waiting to send send request body. 
<li> Waiting for reply header. 
<li> Waiting for reply body. 
<li> Finished. 
</ul>
<p>
FSMs of a particular type are managed by an <b>FSM container</b>.
Each FSM container manages a set of FSMs, and provides a <b>poll()</b>
function for detecting and performing state transitions.
These functions are nonblocking; at the lowest level, they must
use non-blocking network sockets, accessed using select(). 
<p>
The core client uses the following FSM types: 
<ul>
<li>
<b>NET_XFER</b> (container: <b>NET_XFER_SET</b>).
Each instance represents a network connection,
for which data is being transferred to/from memory or a disk file.
The <b>poll()</b> function uses
<b>select()</b> to manage the FSM without blocking. 
<li>
<b>HTTP_OP</b> (container: <b>HTTP_OP_SET</b>).
Each instance represents an HTTP operation (GET, PUT or POST). 
<li>
<b>FILE_XFER</b> (container: <b>FILE_XFER_SET</b>).
Each instance represents a file transfer (upload or download) in progress. 
<li>
<b>PERS_FILE_XFER</b> (container: <b>PERS_FILE_XFER_SET</b>).
Each instance represents a 'persistent file transfer',
which recovers from server failures and disconnections,
and implements retry and give-up policies.
<li>
<b>SCHEDULER_OP</b>.
There is only one instance.
It encapsulates communication with scheduling servers,
including backoff and retry policies.
<li>
<b>ACTIVE_TASK</b> (container: <b>ACTIVE_TASK_SET</b>).
Each instance represents a running application. 
</ul>
<p>
An FSM may be implemented using other FSMs; for example, FILE_XFER
is implemented using HTTP_OP, which in turn is implemented using NET_XFER. 
";
page_tail();
?>
