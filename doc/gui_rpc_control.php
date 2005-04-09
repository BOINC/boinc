<?php
require_once("docutil.php");
page_head("Access control for GUI RPC");
echo "
Since GUI RPCs can control the BOINC client
(e.g. attaching/detaching projects)
it is important to protect your BOINC client
from unauthorized control.
There are two levels of protection:
<ul>
<li> You can associate a password with the client;
GUI RPCs must be authenticated with this password.
<li> You can restrict RPCs to a limited set of hosts.
</ul>

<h2>Password protection</h2>
<p>
If you place a password in a file <b>gui_rpc_auth.cfg</b>
in your BOINC directory,
GUI RPCs must be authenticated using the password.

<h2>Remote host restriction</h2>
<p>
By default the core client accepts GUI RPCs
only from the same host.

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
