<?php
require_once("docutil.php");
page_head("Result assimilation");
echo "
Projects must create one assimilator program per application.
This is best done by linking the program <b>sched/assimilate.C</b>
with an application-specific function of the form
<pre>
int assimilate_handler(
    WORKUNIT& wu, vector&lt;RESULT>& results, RESULT& canononical_result
);
</pre>
";
page_tail();
?>
