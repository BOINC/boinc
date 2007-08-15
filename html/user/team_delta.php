<?php
require_once("../inc/db.inc");
require_once("../inc/user.inc");
require_once("../inc/team.inc");

$xml = $_GET['xml'];

function show_delta($delta) {
    global $xml;
    $user = lookup_user_id($delta->userid);
    $when = time_str($delta->timestamp);
    $what = $delta->joining?"joined":"quit";
    if ($xml) {
        echo "    <action>
        <user_email>$user->email_addr</user_email>
        <id>$user->id</id>
        <name>$user->name</name>
        <action>$what</action>
        <total_credit>$delta->total_credit</total_credit>
        <when>$when</when>
    </action>
";
    } else {
        echo "<tr>
           <td>$when</td>
            <td>",user_links($user)," ($user->email_addr)</td>
           <td>$what</td>
           <td>$delta->total_credit</td>
           </tr>
        ";
    }
}

db_init();

$user = get_logged_in_user();
$teamid=get_int('teamid');
$team = lookup_team($teamid);
if ($xml) {
    require_once('../inc/xml.inc');
    xml_header();
}

if (!$team || $team->userid != $user->id) {
    if ($xml) {
        xml_error("-1", "Not founder");
    } else {
        error_page("You're not the founder of that team");
    }
}

if ($xml) {
    echo "<actions>\n";
} else {
    page_head("Team history for $team->name");
    start_table();
    echo "<tr>
        <th>When</th>
        <th>User</th>
        <th>Action</th>
        <th>Total credit at time of action</th>
        </tr>
    ";
}
$result = mysql_query("select * from team_delta where teamid=$teamid order by timestamp");
while ($delta = mysql_fetch_object($result)) {
    show_delta($delta);
}
mysql_free_result($result);
if ($xml) {
    echo "</actions>\n";
} else {
    end_table();
    page_tail();
}
?>
