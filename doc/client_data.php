<?
require_once("docutil.php");
page_head("Core client: data structures");
echo "
<p>
The central data types in the core client are:
<pre>
PROJECT
APP
FILE_INFO
APP_VERSION
IO_FILE_DESC
WORKUNIT
RESULT
</pre>
These correspond to the
<a href=create_project.html>abstractions of the same name</a>.
They contain some fields that aren't present on the server side;
e.g., FILE_INFO has a field indicating whether the
file is present on this host.
<p>
Each of these classes has <b>write</b> and <b>parse</b> functions
for converting an instance to or from XML.
In some cases there are different variants depending on
whether the repository of the XML is a scheduling server
or the local client_state.xml file.
<p>
These structures are linked by pointers;
for example, an APP_VERSION has a pointer to its APP.

<pre>PREFS</pre>
This class is a parsed version of the prefs.xml file;
as such it doesn't contain any host-specific information.
It contains a vector of partially-populated PROJECT objects,
and parsed versions of user preferences.

<pre>CLIENT_STATE</pre>
This encapsulates the global variables of BOINC on this host.
It is a parsed version of client_state.xml,
represented as vectors of the basic types.
It also includes some transient variables,
such as sets of finite-state machines.

<h3>Initialization</h3>
<p>
When the core client starts up (CLIENT_STATE::init())
it parses the prefs file, creating a PREFS object.
(If there is no prefs file, it prompts the user for
a project and account ID, and creates one.)
It then copies the vector of PROJECT objects to CLIENT_STATE.
<p>
Next, it parses the client_state.xml file.
Any projects in this file but not in prefs.xml are ignored.
As it parses objects that link to other objects,
it looks up the referent (identified by name in XML)
and installs a pointer.

<h3>Handling a scheduler RPC reply</h3>
<p>
A similar approach is used when handling the reply
from a scheduling server RPC (CLIENT_STATE::handle_scheduler_reply).
The RPC returns a interconnected set of basic objects,
represented in a SCHEDULER_REPLY object.
For each of these basic objects, we see (e.g. lookup_app())
if it's already present in the CLIENT_STATE structure.
If not, we create and insert a new object,
then link it (e.g., link_app()) to existing objects in CLIENT_STATE.
";
page_tail();
?>
