<?php

require_once("test.inc");
require_once("../inc/util.inc");
require_once("../inc/db.inc");

$version = get_str("version");
$platform = get_str("platform", true);
$test_group = get_str("test_group", true);
$status = -1;
$status = get_int("status", true);

function show_report($r) {
    $user = lookup_user_id($r->userid);
    $s = array();
    $s[0] = "OK";
    $s[1] = "minor bugs";
    $s[2] = "major bugs";
    $status = $s[$r->status];
    $p = platform_name($r->platform);
    $tg = test_group_name($r->test_group);
    $c = $r->comment;
    if (!$c) $c = "<br>";
    echo "
        <tr>
        <td>$user->name</td>
        <td>$p</td>
        <td>$tg</td>
        <td>$status</td>
        <td>$c</td>
        <td>$r->mod_time</td>
        </tr>
    ";
}

db_init();

$tr = array();
$query = "select * from test_report where version='$version'";
$result = mysql_query($query);
if (!$result) {
    echo mysql_error();
    error_page("db error");
}
while ($r = mysql_fetch_object($result)) {
    array_push($tr, $r);
}
mysql_free_result($result);

echo "
    <table cellpadding=4 border=1>
    <tr>
        <th>User</th>
        <th>Platform</th>
        <th>Test group</th>
        <th>Status</th>
        <th>Comment</th>
        <th>Time</th>
    </tr>
";

for ($i=0; $i<count($tr); $i++) {
    $r = $tr[$i];
    if ($platform && $r->platform != $platform) continue;
    if ($test_group && $r->test_group != $test_group) continue;
    if ($status >= 0 && $r->status != $status) continue;

    show_report($r);
}

echo "</table>\n";
?>
