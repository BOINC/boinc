<?php
require_once("docutil.php");
page_head("Controlling the core client via RPC");
echo "
<p>
The BOINC core client provides a set of RPCs
(remote procedure calls) for control and state interrogation.
This enables the development of GUI (graphical user interface)
programs separately from the core client.
<p>
BOINC provides a C++ interface to these RPCs.
The interface is based on the GUI_RPC class,
which provides the following functions
(the code is in <code>lib/gui_rpc_client.h</code>,
and the program <code>boinc_cmd.C</code> gives a usage example):
<p>
";
list_start();
list_heading("function ", "description");
list_item_func(
    "init(char* host)",
    "Establish RPC connection to the given host"
);
list_item_func(
    "authorize(char* password)",
    "Do authorization sequence with the peer, using given password"
);
list_item_func(
    "get_state(CC_STATE&)",
    "Get the core client's 'static' state,
    i.e. its projects, apps, app_versions, workunits and results.
    This call is relatively slow and should only
    be done initially, and when needed later (see below).
    "
);
list_item_func(
    "get_results(RESULTS&)",
    "Get a list of results.
    Those that are in progress will have information
    such as CPU time and fraction done.
    Each result includes a name;
    use CC_STATE::lookup_result() to find this result in
    the current static state;
    if it's not there, call get_state() again.
    "
);
list_item_func(
    "get_file_transfers(FILE_TRANSFERS&)",
    "Get a list of file transfers in progress.
    Each is linked by name to a project;
    use CC_STATE::lookup_project() to find this project in the current state;
    if it's not there, call get_state() again."
);
list_item_func(
    "get_simple_gui_info(SIMPLE_GUI_INFO&)",
    "Return the list of projects and of active results."
);
list_item_func(
    "get_project_status(vector<PROJECT>&)",
    "Get a list of projects, with only basic fields filled in."
);
list_item_func(
    "get_disk_usage(vector<PROJECT>&)",
    "Get a list of projects, with disk usage fields filled in."
);
list_item_func(
	"show_graphics(char* result_name, bool full_screen)",
	"Request that the application processing the given result
	create a graphics window"
);
list_item_func(
    "project_op(PROJECT&, char* op)",
	"Perform a control operation on the given project.
    <code>op</code> is one of
    \"suspend\",
    \"resume\",
    \"reset\",
    \"detach\", or
    \"update\".
    "
);
list_item_func(
    "project_attach(
    char* url,
    char* account_id,
    bool use_cached_credentials
)",
	"Attach to the given project. Cached credentials are defined in the 
	<a href=client_startup.php>project_init.xml</a> file."
);
list_item_func(
    "set_run_mode(int mode)",
	"Set the run mode (never/auto/always)."
);
list_item_func(
    "get_run_mode(int& mode)",
	"Get the run mode (never/auto/always)."
);
list_item_func(
    "set_network_mode(int mode)",
	"Set the network mode (never/auto/always)."
);
list_item_func(
    "run_benchmarks()",
	"Run benchmarks"
);
list_item_func(
    "set_proxy_settings(PROXY_INFO&)",
	"Set proxy settings"
);
list_item_func(
    "get_proxy_settings(PROXY_INFO&)",
	"Get proxy settings"
);
list_item_func(
    "get_messages(int seqno, MESSAGES&)",
    "Returns a list of messages to be displayed to the user.
    Each message has a sequence number (1, 2, ...),
    a priority (1=informational, 2=error) and a timestamp.
    The RPC requests the messages with sequence numbers greater than 'seqno',
    in order of increasing sequence number."
);
list_item_func(
    "file_transfer_op(FILE_TRANSFER&, char* op)",
	"Perform a control operation on a file transfer.
    <code>op</code> is one of
    \"abort\" or
    \"retry\".
    "
);
list_item_func(
    "result_op(FILE_TRANSFER&, char* op)",
	"Perform a control operation on an active result.
    <code>op</code> is one of
    \"suspend\",
    \"resume\", or
    \"abort\".
    "
);
list_item_func(
    "acct_mgr_rpc(
    const char* url,
    const char* name,
    const char* passwd,
    bool use_cached_credentials
)",
    "Do an <a href=acct_mgt.php>Account Manager RPC</a>
    to the given URL, passing the given name/password.
    If use_cached_credentials is true, then the existing url, username, 
    and password are used and the core client updates the project
    information from the account manager.
    If the RPC is successful, save the account info on disk
    (it can be retrieved later using acct_mgr_info(), see below).
    If URL is the empty string, remove account manager info from disk.
    "
);
list_item_func(
    "acct_mgr_info(ACCT_MGR_INFO&)",
    "Return the URL/name of the current account manager (if any),
    and the user name and password."
);
list_item_func(
    "network_query(int& network_wanted)",
    "Returns a nonzero value in network_wanted if the core client
    wants a network connection or is currently using one.
    Wait to get a false twice (with a few seconds delay)
    before closing a connection."
);
list_item_func(
    "network_available()",
    "Tells the core client that a network connection is available,
    and that it should do as much network activity as it can."
);
list_item_func(
    "read_global_prefs_override()",
    "Tells the core client to reread the 'global_prefs_override.xml' file,
    modifying the global preferences according to its contents."
);
list_end();
echo "
<p>
The RPC mechanism uses XML requests and replies.
It should be easy fairly easy to generate client
interfaces in languages other than C++.
GUI programs connect to the core client by opening a TCP socket at port 31416.
They can then do repeated RPCs over this connection.
Each reply message ends with the character '\\003.


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
a file remote_hosts.cfg in your BOINC directory containing 
a list of allowed DNS host names or IP addresses (one per line).
Those hosts will be able to connect.
The remote_hosts.cfg file can have comment lines that start with either a # 
or a ; character as well.
</ul>
";

page_tail();
?>
