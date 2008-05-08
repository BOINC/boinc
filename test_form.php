<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("test.inc");

db_init();

$user = get_logged_in_user();

page_head("Report test results");
echo "
    Use this form for reporting test results
    for the BOINC version currently under test.
    Fill out this form separately for each platform you tested.
    <p>
    To report bugs with public releases of BOINC,
    use the <a href=http://boinc.berkeley.edu/trac/>BOINC Trac bug database</a>.
    <p>
    <form action=test_form2.php>
    Version:
";
    show_version_select();
echo "
    <p>
    Which platform did you test on?
";
    show_platform_select();
echo "
    <p>
    <input type=submit value='OK'>
    </form>
";

page_tail();
?>
