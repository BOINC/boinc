<?php

require_once("../inc/profile.inc");

db_init();

function buttons($i) {
    echo "
        skip: <input type=radio name=user$i value=0>
        <br>accept: <input type=radio name=user$i value=1>
        <br>reject: <input type=radio name=user$i value=-1>
    ";
}

echo "<form method=get action=profile_screen_action.php>\n";

$result = mysql_query("select * from profile, user where profile.userid=user.id and (uotd_time is null) and (has_picture>0) and (verification=0) and (user.total_credit>0) order by recommend desc limit 20");

$n = 0;
page_head("screen profiles");
start_table();
while ($profile = mysql_fetch_object($result)) {
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
    echo "<input type=hidden name=userid$n value=$profile->userid>\n";
    $n++;
}

end_table();
echo "
    <input type=hidden name=n value=$n>
    <input type=submit value=OK>
    </form>
";
page_tail();
?>
