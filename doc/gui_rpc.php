<?php
require_once("docutil.php");
page_head("Controlling the core client via RPC");
echo "
<p>
The BOINC core client provides a set of RPCs
(remote procedure calls) for control and state interrogation.
This enables the development of GUI (graphical user interface)
programs separately from the core client.
These RPCs send XML request and reply messages over a TCP connection.
The XML formats are not documented, but can be deduced from the source code.
<p>
BOINC provides a C++ interface to these RPCs,
consisting of the <code>GUI_RPC</code> class.
The interface is in <code>lib/gui_rpc_client.h</code>,
and the program <code>boinc_cmd.C</code> gives a usage example).
All member functions return an integer error code.
To create an RPC connection, call

";
list_start();
list_item_func(
    "init(char* host)",
    "Establish RPC connection to the given host"
);
list_end();
echo "
<h2>Dealing with versions</h2>
<p>
The GUI RPC protocol changes over time.
If you're writing a GUI program that needs to communicate with
older versions of the BOINC core client,
here's what to do:
<ul>
<li> Create a GUI_RPC object and connect.
Call exchange_versions() to get the client version.
<li>
If exchange_versions() fails (it's not suppored in pre-5.6 clients)
do a get_state() RPC, and get the client version from CC_STATE::version_info.
<li>
Use the client version number to decide what subsequent RPCs to make
(version info is included in the RPC list below).
</ul>

<h2>Authorization</h2>
<p>
GUI RPC authorization is described <a href=gui_rpc_auth.php>here</a>.
The RPC protocol allows the GUI program to authenticate itself using a password.
Some of the RPC operations can be done without authentication;
others can be done without authentication, but only by a GUI program
running on the same machine.
";
list_start();
list_item_func(
    "authorize(char* password)",
    "Do authorization sequence with the peer, using given password"
);
list_end();
echo "
<p>
<h2>RPC list</h2>
The following functions require authorization for remote clients,
but not for local clients.
Note: for core client versions 5.5.12 and earlier,
all functions except get_state(), get_results(),
get_screensaver_mode(), and set_screensaver_mode() require authorization.
";
list_start();
list_item_func(
    "get_state(CC_STATE&)",
    "Get the core client's 'static' state,
    i.e. its projects, apps, app_versions, workunits and results.
    This call is relatively slow and should only
    be done initially, and when needed later (see below).
    "
);
list_item_func(
    "exchange_versions(VERSION_INFO&)",
    "Exchange version info with the core client.
    The core client's version info is returned."
);
list_item_func(
    "get_cc_status(CC_STATUS&);
struct CC_STATUS {
    int network_status;
    bool ams_password_error;
    int task_suspend_reason;
    int network_suspend_reason;
    int task_mode;
    int network_mode;
    int task_mode_perm;
    int network_mode_perm;
    double task_mode_delay;
    double network_mode_delay;
};
    ",
    "Return a structure containing the network status,
    a flag if there was an account manager password error,
    and data about task and network suspension."
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
    "get_screensaver_mode(int& status)",
    "Return screensaver mode (values listed in ss_logic.h)"
);
list_item_func(
    "set_screensaver_mode(
    bool enabled, double blank_time, DISPLAY_INFO&
)",
    "If enabled is true, the core client should try to get
    an application to provide screensaver graphics.
    Blank screen after blank_time seconds.
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
    "get_proxy_settings(PROXY_INFO&)",
	"Get proxy settings"
);
list_item_func(
    "get_activity_state(ACTIVITY_STATE&)",
    "Return bitmap of reasons why computation and network are suspended.
    Deprecated - for 5.5.13 and later, use cc_status() instead.
    In 5.5.10 and earlier, it returns bool (suspended) rather than bitmap.
    "
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
    "get_host_info(HOST_INFO&)",
    "Get information about host hardware and usage"
);
list_item_func(
    "get_statistics(PROJECTS&)",
    "Get information about project credit history
    (the PROJECT::statistics field is populated)"
);
list_item_func(
    "network_status(int&)",
    "Find whether the core client has, needs, or is done with
    a physical network connection.
    Deprecated - for 5.5.13 and later, use cc_status() instead.
    "
);
list_item_func(
    "get_newer_versions(std::string&)",
    "Get a string describing newer versions of the core client, if any."
);
list_end();

echo "
    The following operations require authentication
    for both local and remote clients:
";

list_start();
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
    bool use_config_file
)",
	"Attach to the given project.
    If 'use_config_file' is true,
    use the project URL (and account key, if present) in the 
	<a href=client_startup.php>project_init.xml</a> file."
);
list_item_func(
    "set_run_mode(int mode, double duration)",
	"Set the run mode (never/auto/always).
    If duration is zero, mode is permanent.
    Otherwise revert to last permanent mode
    after duration seconds elapse."
);
list_item_func(
    "set_network_mode(int mode, double duration)",
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
    "network_available()",
    "Tells the core client that a network connection is available,
    and that it should do as much network activity as it can."
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
    "quit()",
    "Tell the core client to exit."
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
    "read_global_prefs_override()",
    "Tells the core client to reread the 'global_prefs_override.xml' file,
    modifying the global preferences according to its contents."
);
list_item_func(
    "get_global_prefs_override(std::string&)",
    "Reads the contents of the
    <a href=prefs_override.php>global preferences override file</a>
    into the given string.
    Return an error code if the file is not present."
);
list_item_func(
    "set_global_prefs_override(std::string&)",
    "Write the given contents to the
    <a href=prefs_override.php>global preferences override file</a>.
    If the argment is an empty string, delete the file.
    Otherwise the argument must be a valid
    &lt;global_preferences&gt; element."
);
list_item_func(
    "get_global_prefs_override_struct(GLOBAL_PREFS&)",
    "Return the contents of the
    <a href=prefs_override.php>global preferences override file</a>,
    parsed into a structure.
    Default values are zero.
    Returns an error code if the file is not present."
);
list_item_func(
    "set_global_prefs_override_struct(GLOBAL_PREFS&)",
    "Convert the given structure to XML and write it to the
    <a href=prefs_override.php>global preferences override file</a>."
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


";

page_tail();
?>
