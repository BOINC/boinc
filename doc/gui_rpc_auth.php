<?php
require_once("docutil.php");
page_head("Controlling BOINC remotely");
echo "
<h2>Remote control of the BOINC client</h2>
<p>
The BOINC core client typically is controlled by
the <a href=manager.php>BOINC Manager</a> running on the same machine:
<br> <center>
<img src=gui_auth.png>
<br> </center>
The two programs communicate over a local connection,
using 'GUI RPC' (Graphical User Interface Remote Procedure Call).
<p>
It's also possible to use the BOINC Manager to control
a core client on a different host:
<br> <center>
<img src=gui_auth2.png>
<br> </center>
You can use <a href=addons.php>add-on programs</a>
such as BOINCView to control many BOINC clients at once:
<br> <center>
<img src=gui_auth3.png>
<br> </center>

<h2>Access control for GUI RPC</h2>
<p>
Since GUI RPCs can control the BOINC client
(e.g. attaching/detaching projects)
it is important to protect your BOINC client from unauthorized control.
There are two levels of protection:
<ul>
<li>
GUI RPCs are authenticated with a <b>GUI RPC password</b>.
This is stored with the client in the file <b>gui_rpc_auth.cfg</b>.
When BOINC first runs, it generates a long, random password.
You can change it if you like.
<li> You can specify a set of hosts from which RPCs are allowed.
By default, RPCs are allowed only from the same host.
</ul>
A GUI RPC is handled only if it passes both levels of protection.

<h2>Allowing RPCs from remote hosts</h2>
<p>
By default the core client accepts GUI RPCs only from the same host.
You can allow remote hosts to control a core client in two ways:
<ul>
<li> If you run the client with the
<code>-allow_remote_gui_rpc</code> command line option,
it will accept connections from any host
(subject to password authentication).
<li>
You can create
a file <b>remote_hosts.cfg</b> in your BOINC directory containing 
a list of allowed DNS host names or IP addresses (one per line).
These hosts will be able to connect.
The remote_hosts.cfg file can have comment lines that start with either a # 
or a ; character as well.
</ul>
";
page_tail();
?>
