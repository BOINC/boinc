<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("test.inc");

db_init();
page_head("Alpha testers");
echo "
    <p>
    This table shows the alpha testers,
    the platforms they have,
    and the versions for which they have reported results.
    <p>
";

function os_list($user) {
    $r = mysql_query("select distinct os_name from host where userid=$user->id");
    while ($row = mysql_fetch_row($r)) {
        if ($x) $x .= "<br>$row[0]";
        else $x = $row[0];
    }
    if (!$x) $x='---';
    return $x;
}

$r = mysql_query("select * from test_report where status<>3");
while ($tr = mysql_fetch_object($r)) {
    $uarr[$tr->userid][$tr->version]++;
}

echo "<table cellpadding=4 border=1><tr>
    <th>User</th>
    <th>Operating systems</th>
";
for ($i=0; $i<count($versions); $i++) {
    echo "<th>$versions[$i]</th>\n";
}
echo "</tr>\n";
$r = mysql_query("select * from user");
while ($user = mysql_fetch_object($r)) {
    echo "<tr><td>$user->name</td>
        <td>".os_list($user)."</td>
    ";
    for ($i=0; $i<count($versions); $i++) {
        $v = $versions[$i];
        $n = $uarr[$user->id][$v];
        if (!$n) $n='---';
        echo "<td>$n</td>\n";
    }
    echo "</tr>";
}
echo "</table>\n";
page_tail();

?>
