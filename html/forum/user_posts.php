<?php
require_once('../util.inc');
require_once('../time.inc');
require_once('forum.inc');

db_init('../');

$userid = $_GET['userid'];

$user = lookup_user_id($userid);

page_head("Posts by $user->name");
$result = mysql_query("select * from post where user=$userid order by id desc");
$n = 1;
echo "<table>\n";
while($post = mysql_fetch_object($result)) {
    show_post2($post, $n);
    $n++;
}
echo "</table>\n";
mysql_free_result($result);

?>
