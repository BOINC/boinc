<?php

require_once("../inc/forum.inc");
require_once("../inc/time.inc");

db_init();

$search_string = $_GET['search_string'];
$offset = $_GET['offset'];
if (!$offset) $offset=0;
$count = 10;
$what = '';

if ($_GET['titles']) {
    $what = 'titles=1';
    page_head("Titles containing '$search_string'");
    $q = "select * from thread where match(title) against ('$search_string') order by create_time desc limit $offset,$count";
    $result = mysql_query($q);
    echo "<table>";
    $n = 0;
    while ($thread = mysql_fetch_object($result)) {
        show_thread($thread, $n+$offset+1);
        $n += 1;
    }
    echo "</table>";
    mysql_free_result($result);

    if ($offset==0 && $n==0) {
        echo "No titles found containing '$search_string'";
    }
}

if ($_GET['bodies']) {
    if (! empty ($what)) {
        $what .= '&';
    }
    $what .= 'bodies=1';

    page_head("Messages containing '$search_string'");
    $q = "select * from post where match(content) against ('$search_string') order by timestamp desc limit $offset,$count";
    $result = mysql_query($q);
    echo mysql_error();
    echo "<table>";
    $n = 0;
    while ($post = mysql_fetch_object($result)) {
        show_post2($post, $n+$offset+1);
        $n += 1;
    }
    echo "</table>";
    mysql_free_result($result);
    if ($offset==0 && $n==0) {
        echo "No messages found containing '$search_string'";
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
