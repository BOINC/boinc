<?php
require_once('db.inc');
require_once('util.inc');
db_init();
page_head('XML statistics data');

echo "
    <h2>XML statistics data</h2>
    Information about teams, users and hosts
    is available as compressed XML files.
    <p>
    The format is described
    <a href=http://boinc.berkeley.edu/db_dump.php>here</a>.
    <p>
    The files are
    <a href=stats/>here</a>.
";

page_tail();
?>
