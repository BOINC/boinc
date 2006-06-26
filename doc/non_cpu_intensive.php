<?php
require_once("docutil.php");
page_head("Non-CPU-intensive applications");
echo "
A 'non-CPU-intensive' project is one whose applications
use little CPU time.
Examples include:
<ul>
<li> Host measurements
<li> Network measurements
<li> Web crawling
<li> Network data access
</ul>
A non-CPU-intensive project is treated specially by the core client:
<ul>
<li> The core client maintains one result for the project
<li> The core client executes this result whenever computation is enabled,
    bypassing the normal CPU scheduling mechanism.
</ul>
<p>
A project labels itself as non-CPU-intensive by including
the &lt;non_cpu_intensive&gt; flag in its
<a href=configuration.php>configuration file</a>
<p>
This attribute is at the project level;
there is no provision for a project to have some
applications that are CPU intensive and some that aren't.
<p>
Non-CPU-intensive applications can use chunks CPU time;
this won't break anything, and the CPU scheduler will
adjust to it by changing its estimates of 'CPU efficiency'.
However, non-CPU-intensive applications should try not
to use more CPU time than their resource share fraction indicates.
";

page_tail();
?>
