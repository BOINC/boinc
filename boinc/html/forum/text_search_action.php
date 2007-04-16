<?php

require_once("../inc/forum.inc");
require_once("../inc/time.inc");

$search_string = $_GET['search_string'];
$offset = $_GET['offset'];
if (!$offset) $offset=0;
$count = 10;

page_head("Search results");

if ($_GET['titles']) {
    echo "<h2>Titles containing '$search_string'</h2>\n";
    $q = "select * from thread where match(title) against ('$search_string') limit $offset,$count";
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
    echo "<h2>Messages containing '$search_string'</h2>\n";
    $q = "select * from post where match(content) against ('$search_string') limit $offset,$count";
    $result = mysql_query($q);
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
        <a href=text_search_action.php?bodies=1&search_string=$s&offset=$offset>Next $count</a>
    ";

}

page_tail();
?>
