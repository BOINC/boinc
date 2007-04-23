<?php

require_once("docutil.php");

page_head("The project configuration file [deprecated - wiki]");
echo "<!-- \$Id$ -->\n";

echo"
A project's configuration is described by a file
named <b>config.xml</b> in the project's directory.
This file is created, with default values, by the
<a href=make_project.php>make_project</a> script.
However, you will need to change or add some items
during the life of your project.
<p>
The format of config.xml file is:
", html_text("
<boinc>
  <config>
    [ configuration options ]
  </config>
  <daemons>
    [ list of daemons ]
  </daemons>
  <tasks>
    [ list of periodic tasks ]
  </tasks>

</boinc>
"),"
Details on:
<ul>
<li> <a href=project_options.php>Configuration options</a>
<li> <a href=project_daemons.php>Daemons</a>
<li> <a href=project_tasks.php>Periodic tasks</a>
</ul>
";

page_tail();
?>
