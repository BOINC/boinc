<?php
require_once('../inc/util.inc');
require_once('../inc/time.inc');
require_once('../inc/forum.inc');

$userid = $_GET['userid'];
$offset = $_GET['offset'];
if (!$offset) $offset=0;
$count = 10;

$user = lookup_user_id($userid);

page_head("Posts by $user->name");
echo "<h2>Posts by $user->name</h2>\n";
$result = mysql_query("select * from post where user=$userid order by id desc limit $offset,$count");
$n = 0;
start_table();
while($post = mysql_fetch_object($result)) {
    show_post2($post, $n+$offset+1);
    $n++;
}
echo "</table>\n";
mysql_free_result($result);

if ($n == $count) {
    $offset += $count;
    echo "
        <br><br>
        <a href=user_posts.php?userid=$userid&offset=$offset><b>Next $count posts</b></a>
    ";
}

page_tail();
?>
