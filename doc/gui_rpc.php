<?
require_once("docutil.php");
page_head("Framework for separate GUIs");
echo "
<b>NOTE: the following is under development,
and is currently available only with Unix core clients.</b>
<p>
We are moving to a model where the GUI (graphical user interface)
and the core client are separate programs,
communicating using RPC over local pipes.
This will make it easy for people to develop new GUIs,
and will eliminate security issues related
to having GUI code in the core client.
<p>
GUI programs connect to the core client by opening
a Unix-domain socket named <b>gui_rpc</b>
in the core client directory.
They can then do repeated RPCs over this connection.
<p>
Currently only one RPC is defined.
The request message is
<pre>", htmlspecialchars("
<get_state/>
"), "</pre>
and the reply message is an XML document describing
the state of the core client.
This has the same format as the client_state.xml file.
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
The project <code>gui_test.C</code> shows how to user this mechanism.
";
page_tail();
?>
