<?php

require_once("../inc/forum.inc");
require_once("../inc/time.inc");
require_once("../inc/translation.inc");

db_init();

$search_string = stripslashes(get_str('search_string'));
$search_string_clean = mysql_real_escape_string($search_string);
$offset = get_int('offset',true);
if (!$offset) $offset=0;
$count = 10;
$what = '';

if (isset($_GET['titles'])) {
    $what = 'titles=1';
    page_head(sprintf(tr(FORUM_SEARCH_TITLES),$search_string)); //"les containing '$search_string'");
    $q = "SELECT * FROM `thread` WHERE `title` LIKE '%$search_string_clean%' AND `hidden` = 0 ORDER BY `create_time` DESC LIMIT $offset,$count";
    $result = mysql_query($q);
    echo mysql_error();
    echo "<table>";
    $n = 0;
    if (mysql_num_rows($result) > 0) {
        echo "<table>";
        $n = 0;
        while ($n < mysql_num_rows($result)) {
            $thread = mysql_fetch_object($result);
            show_thread($thread, $n+$offset+1);
            $n += 1;
	}
        echo "</table>";
    }
    mysql_free_result($result);

    if ($offset==0 && $n==0) {
        echo sprintf(tr(FORUM_SEARCH_TITLES_NO),$search_string);//"No titles found containing '$search_string'";
    }
    
} else if (isset($_GET['bodies'])) {
    $what .= 'bodies=1';

    page_head(sprintf(tr(FORUM_SEARCH_BODIES),$search_string));
    $q  = "SELECT * FROM post ";
    $q .= "LEFT JOIN thread ON post.thread = thread.id ";
    $q .= "WHERE MATCH(post.content) AGAINST ('$search_string_clean') ";
    $q .= "AND post.hidden = 0 AND thread.hidden = 0 ";// Don't show a post if it is in a hidden thread or hidden
    $q .= "ORDER BY post.timestamp DESC LIMIT $offset,$count";
    $result = mysql_query($q);
    if (mysql_num_rows($result) > 0) {
        echo "<table>";
        $n = 0;
        while ($n < mysql_num_rows($result)) {
            $post = mysql_fetch_object($result);
            show_post2($post, $n+$offset+1);
            $n += 1;
        }
        echo "</table>";    
    }
    mysql_free_result($result);
    if ($offset==0 && $n==0) {
        echo sprintf(tr(FORUM_SEARCH_BODIES_NO),$search_string);
    }
}

if ($n==$count) {
    $s = urlencode($search_string);
    $offset += $count;
    echo "
        <a href=forum_text_search_action.php?$what&search_string=$s&offset=$offset>Next $count</a>
    ";

}

page_tail();
?>
