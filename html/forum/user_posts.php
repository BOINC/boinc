<?php
require_once('../util.inc');
require_once('../time.inc');
require_once('forum.inc');

db_init('../');

$userid = $_GET['userid'];
$offset = $_GET['offset'];
if (!$offset) $offset=0;
$count = 10;

$user = lookup_user_id($userid);

page_head("Posts by $user->name");
$result = mysql_query("select * from post where user=$userid order by id desc limit $offset,$count");
$n = 0;
echo "<table>\n";
while($post = mysql_fetch_object($result)) {
    show_post2($post, $n+$offset+1);
    $n++;
}
echo "</table>\n";
mysql_free_result($result);

if ($n == $count) {
    $offset += $count;
    echo "
        <a href=user_posts.php?userid=$userid&offset=$offset>Next $count</a>
    ";
}
?>
