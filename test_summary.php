<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("test.inc");

db_init();

page_head("Test results summaries");

echo "
    <p>
    This table shows the number of test reports that have been
    submitted for each combination of (platform, version)
    and (test, version).
    <p>
";

function pcount($p, $v) {
    $r = mysql_query("select count(*) from test_report where version='$v' and platform = '$p' and status<>3");
    $s = mysql_fetch_row($r);
    return $s[0];
}
function tcount($p, $v) {
    $r = mysql_query("select count(*) from test_report where version='$v' and test_group = '$p' and status<>3");
    $s = mysql_fetch_row($r);
    return $s[0];
}

$r = mysql_query("select * from test_report");
while ($tr = mysql_fetch_object($r)) {
    $parr[$tr->platform][$tr->version][$tr->status]++;
    $tarr[$tr->test_group][$tr->version][$tr->status]++;
}

echo "
    <table border=1 cellpadding=4>
    <tr>
    <td>Version<br><font size=-2>click for details</td>
    <td>Target<br>#results</td>
";

$max_versions = 12;
for ($i=0; $i<count($versions)&&$i<$max_versions; $i++) {
    echo "
        <td><a href=test_details.php?version=$versions[$i]>$versions[$i]</a></td>
    ";
}
echo "</tr>";

$nc = count($versions)+2;
echo "<tr><th colspan=$nc>PLATFORMS</td></tr>\n";
for ($i=0; $i<count($platforms); $i++) {
    $p = $platforms[$i][0];
    $pl = $platforms[$i][1];
    $tr = $platforms[$i][2];
    echo "<tr><td>$pl</td><td>$tr</td>";
    for ($j=0; $j<count($versions)&&$j<$max_versions; $j++) {
        $v = $versions[$j];
        $x1 = pcount($p, $v);
        echo "<td>$x1</td>\n";
    }
    echo "</tr>\n";
}
echo "<tr><th colspan=$nc>TESTS</td></tr>\n";
for ($i=0; $i<count($test_groups); $i++) {
    $p = $test_groups[$i][0];
    $pl = $test_groups[$i][1];
    $tr = $test_groups[$i][2];
    echo "<tr><td>$pl</td><td>$tr</td>";
    for ($j=0; $j<count($versions)&&$j<$max_versions; $j++) {
        $v = $versions[$j];
        $x1 = tcount($p, $v);
        echo "<td>$x1</td>\n";
    }
    echo "</tr>\n";
}

echo "</table>";

page_tail();
?>
