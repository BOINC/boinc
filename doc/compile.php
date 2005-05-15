<?php
require_once("docutil.php");

page_head("Compiling BOINC software");
echo "
Cookbooks:
<ul>
<li> <a href=mac_build.html>Building BOINC and BOINC applications on Mac OS X</a>
<li> <a href=http://noether.vassar.edu/~myers/help/boinc/boinc-on-windows.html>Building BOINC applications on Windows</a>
<li> <a href=http://noether.vassar.edu/~myers/help/boinc/boinc-on-linux.html>Building BOINC and BOINC Applications on Linux</a>
</ul>

More in-depth:
<ul>
<li> <a href=source_code.php>Getting the source code</a>
<li> <a href=build.php>Software prerequisites</a>
<li> <a href=road_map.php>Road map of the BOINC software</a>
<li> <a href=build_system.php>Build system</a>
<li> <a href=build_server.php>Building server components</a>
<li> <a href=build_client.php>Building the core client</a>
<li> <a href=test.php>Test applications and scripts</a>
<li> <a href=ssl_build.txt>Build instructions for SSL (Secure Socket Layer) client</a>
</ul>
";
page_tail();

?>
