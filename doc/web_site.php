<?
require_once("docutil.php");
page_head("The master URL");
echo "
<p>
Each project is publicly identified by a <b>master URL</b>.
The URL cannot change, so choose it carefully.
The <b>master page</b> at this URL has two functions.
<ul>
<li> It is the home page of the project; when viewed in a browser it
describes the project and contains links for registering and for
downloading the core client.
<li> It contains XML elements of the form
<pre>
";
echo htmlspecialchars("
<scheduler>http://host.domain.edu/cgi/scheduler</scheduler>
<scheduler>http://host2.domain.edu/cgi/scheduler</scheduler>
");
echo"
</pre>
that give the URLs of the project's scheduling servers.
These tags can be embedded within HTML comments.
The BOINC core client reads
and parses the master page to locate scheduling servers.
If at any point it is unable to
connect to any scheduling server for a project,
it rereads the master page.
This mechanism lets a project change the locations of its scheduling
servers, or add new servers. 
</ul>
";
page_tail();
?>
