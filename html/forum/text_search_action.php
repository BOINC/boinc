<?php

require_once("forum.inc");
require_once("../time.inc");

$search_string = $_GET['search_string'];
$offset = $_GET['offset'];
if (!$offset) $offset=0;
$count = 10;

page_head("Search results");

function show_thread($thread, $n) {
    $forum = getForum($thread->forum);
    $category = getCategory($forum->category);
    $first_post = getFirstPost($thread->id);
    $title = stripslashes($thread->title);
    $where = $category->is_helpdesk?"Questions and answers":"Message boards";
    $top_url = $category->is_helpdesk?"help_desk.php":"index.php";
    $excerpt = sub_sentence(stripslashes($first_post->content), ' ', EXCERPT_LENGTH, true);
    $posted = time_diff_str($thread->create_time, time());
    $last = time_diff_str($thread->timestamp, time());
    $m = $n%2;
    echo "
        <tr class=row$m>
        <td><font size=-2>
            $n) Posted $posted
            <br>
            Last response $last
        </td>
        <td valign=top>
            <a href=$top_url>$where</a> : $category->name :
            <a href=forum.php?id=$forum->id>$forum->title</a> :
            <a href=thread.php?id=$thread->id>$title</a>
            <br>
            <font size=-2>$excerpt</font>
        </td>
        </tr>
    ";
}

function show_post2($post, $n) {
    $thread = getThread($post->thread);
    $forum = getForum($thread->forum);
    $category = getCategory($forum->category);
    $where = $category->is_helpdesk?"Questions and answers":"Message boards";
    $top_url = $category->is_helpdesk?"help_desk.php":"index.php";
    $content = nl2br(stripslashes($post->content));
    $when = time_diff_str($post->timestamp, time());
    $user = lookup_user_id($post->user);
    $title = stripslashes($thread->title);
    $m = $n%2;
    echo "
        <tr class=row$m>
        <td>
            $n) <a href=$top_url>$where</a> : $category->name :
            <a href=forum.php?id=$forum->id>$forum->title</a> :
            <a href=thread.php?id=$thread->id>$title</a>
            <br>
            Posted $when by $user->name
            <hr>
            $content
        </td>
        </tr>
    ";
}

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
