<?php
require_once("docutil.php");

page_head("Creating work on demand");
echo "
<p>
The daemon program
<pre>
make_work -wu_name name -cushion N
</pre>
creates copies of the given 'master workunit'
as needed to maintain a supply of at least N unsent results.
This is useful for testing purposes.
<p>
Note: if you run the file_deleter and/or db_purge,
the master workunit or its input files may be deleted
(which will cause make_work to fail).
To avoid this, give the master workunit a name that contains
'nodelete' as a substring.
This causes the file_deleter and db_purge to skip over it.

<p>
It may be convenient to have a script that recreates
the master workunit.
For example:

<pre>
cp test_workunits/12ja04aa `bin/dir_hier_path 12ja04aa`
bin/create_work -appname setiathome -wu_name sah_nodelete -wu_template templates/setiathome_wu_0 -result_template templates/setiathome_result_0 12ja04aa
</pre>

";

page_tail();
?>
