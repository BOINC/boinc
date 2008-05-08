<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");

require_once("test.inc");

db_init();

$user = get_logged_in_user();
$platform = process_user_text(get_str("platform"));
$version = process_user_text(get_str("version"));

for ($i=0; $i<count($test_groups); $i++) {
    $t = $test_groups[$i];
    $sname = $t[0]."_status";
    $cname = $t[0]."_comment";

    $status = null;
    $comment = null;
    $status = get_str($sname, true);
    if (is_null($status)) continue;
    $status = process_user_text($status);
    $comment = process_user_text(get_str($cname, true));

    $query = "select * from test_report where userid=$user->id and version='$version' and platform='$platform' and test_group='$t[0]'";
    $result = mysql_query($query);
    $tr = mysql_fetch_object($result);
    if ($tr) {
        $query = "update test_report set status=$status, comment='$comment' where userid=$user->id and version='$version' and platform='$platform' and test_group='$t[0]'";
        $retval = mysql_query($query);
        echo "<br>$t[1]: updating existing report\n";
    } else {
        echo "inserting";
        $query = "insert into test_report (userid, version, platform, test_group, status, comment) values ($user->id, '$version', '$platform', '$t[0]', $status, '$comment')";

        $retval = mysql_query($query);
        echo "<br>$t[1]: adding new report\n";
    }
    if (!$retval) {
        echo mysql_error();
        error_page("db error");
    }
}

echo "
    <p>
    Test report accepted - thank you.
    <p>
    <a href=test_form.php>Submit more test results</a>
";
?>
