<?
require_once("docutil.php");
page_head("Server details");

echo "
Normally the <a href=make_project.php>make_project</a> script
is used to create a server complex.
However, this script can only create server components on a single host.
If you want distribute or replicated server components
you'll need to do some additional work.

<ul>
<li> <a href=sched_server_setup.php>Setting up a scheduling server</a>
<li> <a href=key_setup.php>Creating encryption keys</a>
<li> <a href=database_setup.php>Setting up the BOINC database</a>
<li> <a href=data_server_setup.php>Setting up a data server</a>
<li> <a href=project_startup.php>Project startup checklist</a>
</ul>
";
page_tail();
?>
