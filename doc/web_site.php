<?
require_once("docutil.php");
page_head("Setting up a web site");
echo "
<p>
<h3>The master URL</h3> 
<p>
Each project is publicly identified by a <b>master URL</b>.
The <b>master page</b> at this URL has two functions.
<ul>
<li> It is the home page of the project; when viewed in a browser it
describes the project and contains links for registering and for
downloading the core client.
<li> It contains XML elements of the form <pre>
&lt;scheduler&gt;http://host.domain.edu/cgi/scheduler&lt;scheduler&gt;
&lt;scheduler&gt;http://host2.domain.edu/cgi/scheduler&lt;scheduler&gt;
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

<h3>Database name</h3>
<p>
The web directories html_user/ there is a file called db_name containing the
name of the database for that project.
Each BOINC project will have a different database name.
This allows for access to different databases through the webpages of projects.
";
page_tail();
?>
