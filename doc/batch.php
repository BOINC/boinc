<?php
require_once("docutil.php");
page_head("Batches");
echo "
<p>
Workunits and results can be grouped together into <b>batches</b>.
Each batch is represented by an integer.
Results must belong to the same batch as their workunit.
<p>
BOINC provides tools for manipulating workunits and results by
batch: e.g., for changing the status of all results in a batch from
'inactive' to 'unsent'.
";
page_tail();
?>
