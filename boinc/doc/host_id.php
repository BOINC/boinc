<?
require_once("docutil.php");
page_head("Host identification");
echo "
<h3>Host identification</h3> 
<p>
Not all systems have an application-readable globally unique IDs,
so we use a different approach to host identification.
When a host first contacts the scheduling server it is assigned a host ID.
The server also maintains an RPC sequence number for each host.
Both the host Id and the RPC sequence number are
stored in the client's <code>client_state.xml</code> file. 

<p>
If the scheduling server receives an RPC with a sequence number
less than the expected sequence number,
it creates a new host record in the database.
(This might happen, for example, if a user copies
the <code>client_state.xml</code> file between hosts.)

<h3>Merging duplicate host records</h3>
<p>
This mechanism can lead to situations where the server has
multiple host records for a single host.
For example, this will occur if the user deletes
the <code>client_state.xml</code> file.
<p>
The user can merge these duplicates to a single record
using the <b>Merge hosts</b> command on the web interface.

";
page_tail();
?>
