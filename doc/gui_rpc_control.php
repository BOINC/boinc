<?php
require_once("docutil.php");
page_head("Access control");
echo "
By default the core client accepts GUI RPC connections
only from programs on the same host.
You can provides access to other hosts in two ways:
<ul>
<li> If you run the client with the
-allow_remote_gui_rpc command line option,
it will accept connections from any host.
<li>
You can create
a file remote_hosts.cfg in your BOINC directory containing 
a list of allowed dns host names or ip addresses (one per line).
Those hosts will be able to connect.
The remote_hosts.cfg file can have comment lines that start with either a # 
or a ; character as well.
</ul>
";

page_tail();
?>
