<?php
require_once("docutil.php");

page_head("How to submit bug reports and feature requests");
echo "
There are several mechanisms for feedback on BOINC
(bug reports, feature requests, etc.).

<ul>

<li> Post a message to the appropriate
<a href=email_lists.php>email list</a>.
Do this first.
Your bug may have already been fixed.

<li> Use BOINC's
<a href=http://bbugs.axpr.net/>external bug database</a>
(which is readable and writable).

";
if (1) {
    echo "
<li> Read BOINC's
<a href=https://setiathome.berkeley.edu/taskbase>internal bug database</a>.
This is used by BOINC developers,
and is readable by the rest of the world.
</ul>
";
}
?>
