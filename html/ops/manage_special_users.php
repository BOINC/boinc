<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

require_once('../inc/forum.inc');
require_once('../inc/util_ops.inc');

db_init();

admin_page_head('Manage special users');

start_table("align=\"center\"");
row1("Current special users", '9');

echo "<tr><td>User</td>";
for($i=0;$i<=6;$i++) {
    echo "<td width=\"15\">" . $special_user_bitfield[$i] . "</td>\n";
}
echo "</tr>";

$result = mysql_query("SELECT prefs.userid, prefs.special_user, user.name 
                       FROM forum_preferences as prefs, user 
                       WHERE special_user > '0' and prefs.userid=user.id");
for($i=1;$i<=mysql_num_rows($result);$i++){
	$foo = mysql_fetch_object($result);
    echo "<form action=\"manage_special_users_action.php\" method=\"POST\">\n";
    echo "<input type=\"hidden\" name=\"userid\" value=\"$foo->userid\"
          <tr><td>$foo->name</td>";
    for ($j=0;$j<=6;$j++) {
        $bit = substr($foo->special_user, $j, 1);
        echo "<td><input type=\"checkbox\" name=\"".$j."\" value=\"1\"";
        if ($bit == 1) {
            echo " checked=\"checked\"";
        }
        echo "></td>\n";
    }
    echo "<td><input type=\"submit\" value=\"Update\"></form></td>";
    echo "</tr>\n";
}

echo "<tr><form action=\"manage_special_users_action.php\" method=\"POST\">\n";
echo "<td>Add UserID:<input type=\"text\" name=\"userid\" size=\"6\"></td>";

for ($j=0;$j<=6;$j++) {
        echo "<td><input type=\"checkbox\" name=\"".$j."\" value=\"1\"";
        echo "></td>\n";
    }
    echo "<td><input type=\"submit\" value=\"Update\"></form></td>";
    echo "</tr>\n";


end_table();

admin_page_tail();

?>
