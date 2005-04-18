<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

require_once("../inc/profile.inc");
require_once("../inc/util_ops.inc");

db_init();

function buttons($i) {
    echo "
        <input type=\"radio\" name=\"user$i\" value=\"0\"> skip <br>
        <input type=\"radio\" name=\"user$i\" value=\"1\" checked=\"checked\"> accept <br>
        <input type=\"radio\" name=\"user$i\" value=\"-1\"> reject
    ";
}

admin_page_head("screen profiles");

$result = mysql_query("select * from profile, user where profile.userid=user.id and (uotd_time is null) and (has_picture>0) and (verification=0) and (user.total_credit>0) order by recommend desc limit 20");

$n = 0;
echo "<form method=\"get\" action=\"profile_screen_action.php\">\n";
start_table();
$found = false;
while ($profile = mysql_fetch_object($result)) {
    $found = true;
    echo "<tr><td>
    ";
    buttons($n);
    echo "
        </td><td>
    ";
    echo "recommends: $profile->recommend
        <br>rejects: $profile->reject
        <br>
    ";
    show_profile($profile->userid, true);
    echo "</td></tr>\n";
    echo "<input type=\"hidden\" name=\"userid$n\" value=\"$profile->userid\">\n";
    $n++;
}

end_table();

if ($found) {
    echo "
        <input type=\"hidden\" name=\"n\" value=\"$n\">
        <input type=\"submit\" value=\"OK\">
    ";
} else {
    echo "No more profiles to screen.";
}

echo "
    </form>
";

admin_page_tail();
?>
