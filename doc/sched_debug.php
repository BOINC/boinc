<?php
require_once("docutil.php");
page_head("Scheduling server: debugging");
echo "
<pre>

Here's a useful technique for troubleshooting scheduler problems:

1) Copy the \"scheduler_request_X.xml\" file from a client to the
   machine running the scheduler.  (X = your project URL)

2) Run the scheduler under the debugger, giving it this file as stdin,
   i.e.:
 
   gdb cgi
   (set a breakpoint)
   r < scheduler_request_X.xml
                      
3) You may have to doctor the database as follows:
   update host set rpc_seqno=0, rpc_time=0 where hostid=N
   to keep the scheduler from rejecting the request.
</pre>
";
page_tail();
?>
