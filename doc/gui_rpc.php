<?php
require_once("docutil.php");
page_head("Framework for separate GUIs");
echo "
<p>
The BOINC core client provides a set of RPCs
(remote procedure calls) for control and state interrogation.
This will enable the development of GUI (graphical user interface)
programs separately from the core client.
This will make it easier to develop new GUIs,
and will eliminate security issues related
to having GUI code in the core client.
<p>
BOINC provides a C++ interface to these RPCs.
The interface is based on the GUI_RPC class,
which provides the following functions
(the program <code>gui_test.C</code> gives an example of their use):
<p>
";
list_start();
list_heading("function ", "description");
list_item("init(char* host)", "Establish RPC connection to the given host");
list_item("get_state(CC_STATE&)",
    "Get the core client's 'static' state,
    i.e. its projects, apps, app_versions, workunits and results.
    This call is relatively slow and should only
    be done initially, and when needed later (see below).
    "
);
list_item("get_results(RESULTS&)",
    "Get a list of results.
    Those that are in progress will have information
    such as CPU time and fraction done.
    Each result includes a name;
    use CC_STATE::lookup_result() to find this result in
    the current static state;
    if it's not there, call get_state() again.
    "
);
list_item("get_file_transfers(FILE_TRANSFERS&)",
    "Get a list of file transfers in progress.
    Each is linked by name to a project;
    use CC_STATE::lookup_project() to find this project in
    the current state state;
    if it's not there, call get_state() again."
);

list_item(
    "get_messages(int nmessages, int seqno, vector&lt;MESSAGE_DESC>&)",
    "Returns a list of (user-level) messages.
    Each message has a sequence number (1, 2, ...),
    a priority (1=informational, 2=error)
    and a timestamp.
    The RPC requests the N most recent messages
    with sequence numbers greater than M.
    They are returned in order of decreasing sequence number."
);

list_item(
	"show_graphics(char* result_name, bool full_screen)",
	"Request that the application processing the given result
	create a graphics window"
);
list_item(
    "project_reset(char* url)",
	"Reset the given project"
);
list_item(
    "project_update(char* url)",
	"Update the given project"
);
list_item(
    "project_detach(char* url)",
	"Detach from the given project"
);
list_item(
    "project_attach(char* url, char* account_id)",
	"Attach to the given project"
);
list_item(
    "run_benchmarks()",
	"Run benchmarks"
);
list_item(
    "set_proxy_settings(PROXY_INFO&)",
	"Set proxy settings"
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
