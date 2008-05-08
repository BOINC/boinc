<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("test.inc");

db_init();

$user = get_logged_in_user();
$platform = process_user_text(get_str("platform"));
$version = process_user_text(get_str("version"));


page_head("Report test results");

echo "
    <p>
    <form action=test_action.php>
    <input type=hidden name=platform value=$platform>
    <input type=hidden name=version value=$version>
";
show_test_groups($user, $version, $platform);
echo "
    <p>
    <input type=submit value='OK'>
    </form>
";
page_tail();

?>

