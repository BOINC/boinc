<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("test.inc");

db_init();
$version = process_user_text(get_str("version"),true);

function show_status($counts, $url) {
    $colors[0] = "00ff00.gif";
    $colors[1] = "ffff00.gif";
    $colors[2] = "ff0000.gif";
    echo "
        <table border=1>
    ";
    for ($i=0; $i<3; $i++) {
        if ($counts[$i]) {
            $w = $counts[$i]*20;
            echo "
                <tr>
                <td><a href=$url&status=$i><img src=$colors[$i] width=$w height=12></a>$counts[$i]</td>
                </tr>
            ";
        } else {
            echo "<tr><td><br></td></tr>\n";
        }
    }
    echo "
        </table>
    ";
}

page_head("Test status of version $version");

$n = array();
$n[0] = 0;
$n[1] = 0;
$n[2] = 0;

$tr = array();
$query = "select * from test_report where version='$version'";
$result = mysql_query($query);
if (!$result) {
    echo mysql_error();
    error_page("db error");
}
while ($r = mysql_fetch_object($result)) {
    array_push($tr, $r);
    $n[$r->status]++;
}
mysql_free_result($result);

start_table();
row1("OVERALL STATUS");
echo "<tr><td>\n";
show_status($n, "test_list.php?version=$version");
$fl = fraction_left($version);
$pd = number_format(100*(1-$fl), 0);

echo "
    <br>
    $pd% of target testing has been done.
    </td></tr>
";
end_table();

start_table();
row1("PLATFORMS");

for ($i=0; $i<count($platforms); $i++) {
    $p = $platforms[$i];
    $n[0] = 0;
    $n[1] = 0;
    $n[2] = 0;

    for ($j=0; $j<count($tr); $j++) {
        $r = $tr[$j];
        if ($r->platform != $p[0]) continue;
        $n[$r->status]++;
    }

    row1($p[1]);
    echo "<tr><td>\n";
    show_status($n, "test_list.php?version=$version&platform=$p[0]");
    echo "</td></tr>\n";
}
end_table();

start_table();
row1("TEST GROUPS");

for ($i=0; $i<count($test_groups); $i++) {
    $tg = $test_groups[$i];
    $n[0] = 0;
    $n[1] = 0;
    $n[2] = 0;

    for ($j=0; $j<count($tr); $j++) {
        $r = $tr[$j];
        if ($r->test_group != $tg[0]) continue;
        $n[$r->status]++;
    }

    row1($tg[1]);
    echo "<tr><td>\n";
    show_status($n, "test_list.php?version=$version&test_group=$tg[0]");
    echo "</td></tr>\n";
}
end_table();
?>
