<?php
require_once("docutil.php");

page_head("Creating work on demand");
echo "
<p>
The daemon program
<pre>
make_work -wu_name name -cushion N
</pre>
creates copies of the given work unit
as needed to maintain a supply of at least N unsent results.

<p>
This is useful for testing purposes.

";

page_tail();
?>
