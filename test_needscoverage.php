<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("test.inc");

db_init();

page_head("Tests that need coverage");

echo "
    <p>
    This report describes which platform a test groupings still need additional coverage.
    <p>
";

echo "
    <table border=1 cellpadding=4>
";

echo "<tr><th>PLATFORMS</th></tr>\n";
for ($i=0; $i<count($platforms); $i++) {
    $p = $platforms[$i][0];
    $pl = $platforms[$i][1];
    $result_target = $platforms[$i][2];
    $v = $versions[0];
    $r = mysql_query("select count(*) as count from test_report where version='$v' and platform = '$p' and status<>3");
    $l = mysql_fetch_array($r, MYSQL_BOTH);
    $result_count = intval($l[0]);
    if ( $result_count < $result_target ) {
      echo "<tr><td>$pl</td><td>$result_target</td><td>$result_count</td></tr>\n";
    }
}

echo "<tr><th>TESTS</th></tr>\n";
for ($i=0; $i<count($test_groups); $i++) {
    $p = $test_groups[$i][0];
    $pl = $test_groups[$i][1];
    $result_target = $test_groups[$i][2];
    $v = $versions[0];
    $r = mysql_query("select count(*) as count from test_report where version='$v' and test_group = '$p' and status<>3");
    $l = mysql_fetch_array($r, MYSQL_BOTH);
    $result_count = intval($l[0]);
    if ( $result_count < $result_target ) {
      echo "<tr><td>$pl</td><td>$result_target</td><td>$result_count</td></tr>\n";
    }
}

echo "</table>";

page_tail();
?>
