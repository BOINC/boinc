<?
require_once("docutil.php");
page_head("Framework for separate GUIs");
echo "
<b>NOTE: the following is under development,
and is currently available only with Unix core clients.</b>
<p>
The BOINC core client provides a set of RPCs
(remote procedure calls) for control and state interrogation.
This will enable the development of GUI (graphical user interface)
programs separately from the core client.
This will make it easier to develop new GUIs,
and will eliminate security issues related
to having GUI code in the core client.
<p>
GUI programs connect to the core client by opening a TCP socket at port XXX.
They can then do repeated RPCs over this connection.
<p>
The current RPCs are available:
";
list_start();
list_item(
	html_text("<get_state/>"),
	"Get the state of the core client.
	The reply message
	This has the same format as the client_state.xml file."
);
list_item(
	html_text(
		"<result_show_graphics>
    <project_url>X</project_url>
    <result_name>X</result_name>
</result_show_graphics>
	"),
	"Request that the application processing the given result
	create a graphics window"
);
list_item(
	html_text(
		"<project_reset>
    <project_url>X</project_url>
</project_reset>"
	),
	"Reset the given project"
);
list_item(
	html_text(
		"<project_update>
    <project_url>X</project_url>
</project_update>"
	),
	"Update the given project"
);
list_item(
	html_text(
		"<project_detach>
    <project_url>X</project_url>
</project_detach>"
	),
	"Detach from the given project"
);
list_item(
	html_text(
		"<project_attach>
    <project_url>X</project_url>
    <authenticator>X</authenticator>
</project_attach>"
	),
	"Attach to the given project"
);
list_item(
	html_text("<run_benchmarks/>"),
	"Run benchmarks"
);
list_item(
	html_text(
		"<set_proxy_settings>
    <proxy_server_name>X</proxy_server_name>
    <proxy_server_port>X</proxy_server_port>
</set_proxy_settings>"
	),
	"Set proxy settings"
);
list_end();
echo "
<p>
Eventually we will add RPCs allowing the GUI to control
the core client (e.g. to suspend and resume it,
to attach or detach projects, etc.
<p>
The BOINC source code distribution includes files
<code>gui_rpc_client.C</code> and <code>gui_rpc_client.h</code>
in the client directory.
These define a class <code>GUI_RPC</code>
that manages the establishment of the RPC pipe,
and that parses the returned XML into a data structure
from which the GUI can be generated.
The project <code>gui_test.C</code> shows how to use this mechanism.
";
page_tail();
?>
