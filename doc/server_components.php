<?
require_once("docutil.php");
page_head("What is a project?");
echo "

<p>
A BOINC project consists of the following components:
<ul>
<li> A <a href=database.php>database</a>
<li> A <a href=server_dirs.php>directory structure</a>
<li> A <a href=backend_programs.php>set of daemon processes</a>
<li> A <a href=configuration.php>configuration file</a>
</ul>

<p>
Multiple BOINC projects can exist on the same host.
This can be handy for creating separate projects for testing and debugging. 

<p>
The easiest way to create a project
is with the <a href=make_project.php>make_project</a> script,
which creates skeletal versions of the above components.
<p>
A project must be <b>stopped</b> when maintenance activities
(e.g. changing the configuration file or database) are being performed.
This is done using
<a href=tool_start.php>project control scripts</a>.

<h3>The master URL</h3>
<p>
Each project is publicly identified by a <b>master URL</b>.
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
This mechanism lets a project move or add scheduling servers.
</ul>
<p>
<b>make_project</b>
creates a master page in project/html/user/index.php.
This file includes the file 'schedulers.txt',
which contains the list of ", htmlspecialchars("<scheduler>"), "elements.
</ul>
";
page_tail();
?>
