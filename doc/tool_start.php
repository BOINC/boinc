<?php
   require_once("docutil.php");
   page_head("Project control scripts");
echo "
    While a project is disabled, all its server processes
    are stopped, and the web site displays a 'Project down' message.
    No database accesses occur while a project is disabled.
    A project should be disabled during most maintenance functions
    (such as upgrading software).
    <p>
    The following scripts (in bin/) control a project:
    <p>
    <b>start</b>
    <p>
    Set the project to Enabled mode, and start all daemons.
    <p>
    <b>stop</b>
    <p>
    Set the project do Disabled mode, and stop all daemons.
    <p>
    <b>cron (invoked as 'start --cron')</b>
    <p>
    If the project is Enabled, perform all periodic tasks
    that are past due, and start any daemons that aren't running.
    Otherwise do nothing.
    <p>
    <b>status</b>
    <p>
    Show whether the project is in Enabled mode.
    Show the status of all daemons.
    Show the status of all periodic tasks
    (e.g., when they were last executed).
";
   page_tail();
?>

