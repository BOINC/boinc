<?php
require_once("docutil.php");

page_head("Creating busy work");
echo "
<p>
The daemon program
<pre>
busy_work -wu_name name -cushion N
</pre>
creates an endless supply of work.
Specifically, it creates copies of the given work unit
as needed to maintain a supply of at least N unsent results.

<p>
This is useful for testing purposes.

";

page_tail();
?>
