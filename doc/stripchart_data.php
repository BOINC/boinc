<?php
require_once("docutil.php");
page_head("Stripchart data");
echo "

<ul>
<li> CPU load
<li> number of users
<li> number of results
<li> number of results sent
<li> number of results returned
<li> disk usage
<li> connections/second
<li> network bandwidth
</ul>
";
page_tail();
?>
