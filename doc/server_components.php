<?
require_once("docutil.php");
page_head("Server components");
echo "

<h2>Server components</h2>

<p>
A BOINC server consists of the following components:
<ul>
<li> Database server
<li> Scheduling server(s)
<li> Web server(s)
<li> Data server(s)
</ul>
These components may all run on a single host,
or on separate hosts.
Some of them, as indicated, may be replicated.

<p>
These components are described in detail in separate documents,
which are required reading if you want to separate
and/or replicate the components.
To set up a basic server where all components run on a single host,
you can use a simple <a href=make_project.php>server setup script</a>.
";
page_tail();
?>
