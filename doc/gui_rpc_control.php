<?php
require_once("docutil.php");
page_head("Access control for GUI RPC");
echo "
By default the core client accepts GUI RPC connections
only from programs on the same host.

<p>
<b>
NOTE: this means that any user on the same machine
can control the core client.
This is undesirable; we are planning to add
a password-based protection mechanism to GUI RPC.
</b>

<p>
You can allow remote hosts to control a core client in two ways:
<ul>
<li> If you run the client with the
-allow_remote_gui_rpc command line option,
it will accept connections from any host.
This is not recommended unless the host is behind a firewall
that blocks the GUI RPC port (1043).
<li>
You can create
a file remote_hosts.cfg in your BOINC directory containing 
a list of allowed DNS host names or IP addresses (one per line).
Those hosts will be able to connect.
The remote_hosts.cfg file can have comment lines that start with either a # 
or a ; character as well.
</ul>
";

page_tail();
?>
