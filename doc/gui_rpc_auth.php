<?php
require_once("docutil.php");
page_head("Authorizing remote control of BOINC");
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
And BOINCView (an add-on program developed by a third party) is
able to control many BOINC clients at once:
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
<li> You can associate a password with the client.
If a password is used,
GUI RPCs must be authenticated with this password.
<li> You can restrict RPCs to a limited set of hosts.
</ul>
A GUI RPC is handled only if it passes both levels of protection.
<p>
After a standard installation, BOINC is highly secure;
it generates its own (long, random) password,
and it doesn't allow access from other hosts.

<h2>Password protection</h2>
<p>
If you place a password in a file <b>gui_rpc_auth.cfg</b>
in your BOINC directory,
GUI RPCs must be authenticated using the password.
<p>
If this file is not present, there is no password protection.

<h2>Remote host restriction</h2>
<p>
By default the core client accepts GUI RPCs only from the same host.

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
a file <b>remote_hosts.cfg</b> in your BOINC directory containing 
a list of allowed DNS host names or IP addresses (one per line).
Those hosts will be able to connect.
The remote_hosts.cfg file can have comment lines that start with either a # 
or a ; character as well.
</ul>
";
page_tail();
?>
