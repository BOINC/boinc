<?php
require_once("docutil.php");

page_head("Account managers");

echo "
An <b>account manager</b> is a web site that
makes it easy to find and join BOINC projects.
The way this works is described <a href=acct_mgt.php>here</a>.
<p>
Available account managers:
<ul>
<li> The <a href=http://bam.boincstats.com>BOINCStats Account Manager</a>
    (BAM!) 
</ul>
Other account managers are currently under development,
";
page_tail();
?>
