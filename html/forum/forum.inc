<?php

require_once('../db.inc');
require_once('../sanitize_html.inc');

define('NO_CONTROLS', 0);
define('FORUM_CONTROLS', 1);
define('HELPDESK_CONTROLS', 2);
define("EXCERPT_LENGTH", "120");


define ('SOLUTION', 'This answered my question');
define ('SUFFERER', 'I also have this question');
define ('OFF_TOPIC', 'Off-topic');

$forum_sort_styles['modified-new'] = "Most recent post first";
$forum_sort_styles['modified-old'] = "Least recent post first";
//$forum_sort_styles['activity-most'] = "Most recent activity first";
$forum_sort_styles['views-most'] = "Most views first";
$forum_sort_styles['replies-most'] = "Most posts first";

$faq_sort_styles['create_time'] = "Most recent question first";
$faq_sort_styles['timestamp'] = "Most recent answer first";
$faq_sort_styles['activity'] = "Most often asked first";

$answer_sort_styles['score'] = "Highest score first";
$answer_sort_styles['timestamp'] = "Most recent first";

$thread_sort_styles['timestamp'] = "Newest first";
$thread_sort_styles['timestamp_asc'] = "Oldest first";
$thread_sort_styles['score'] = "Highest rated first";

$thread_filter_styles['2'] = "\"Very helpful\"";
$thread_filter_styles['1'] = "At least \"helpful\"";
$thread_filter_styles['0'] = "At least \"neutral\"";
$thread_filter_styles['-1'] = "At least \"unhelpful\"";
$thread_filter_styles['-2'] = "All posts";

$post_ratings['2'] = "Very helpful (+2)";
$post_ratings['1'] = "Helpful (+1)";
$post_ratings['0'] = "Neutral";
$post_ratings['-1'] = "Unhelpful (-1)";
$post_ratings['-2'] = "Off topic (-2)";

db_init('../');

function getCategories() {
    $langID = (!empty($_SESSION['lang']['id']))?$_SESSION['lang']['id']:1;
    $sql = "SELECT * FROM category WHERE lang = ".$langID." AND is_helpdesk = 0 ORDER BY orderID ASC";
    return mysql_query($sql);
}

function getHelpDeskCategories() {
    $sql = "SELECT * FROM category WHERE is_helpdesk = 1 ORDER BY orderID ASC";
    return mysql_query($sql);
}

function getForums($categoryID) {
    $sql = 'SELECT * FROM forum WHERE category = ' . $categoryID . ' ORDER BY orderID ASC';
  return mysql_query($sql);
}

function getThreads($forumID, $min=-1, $nRec=-1, $sort_style='modified-new') {
    $sql = 'SELECT * FROM thread WHERE forum = ' . $forumID;
    switch($sort_style) {
        case 'modified-new':
            $sql .= ' ORDER BY timestamp DESC';
            break;
        case 'modified-old':
            $sql .= ' ORDER BY timestamp ASC';
            break;
        case 'views-most':
            $sql .= ' ORDER BY views DESC';
            break;
        case 'replies-most':
            $sql .= ' ORDER BY replies DESC';
            break;
        case 'create_time':
            $sql .= ' ORDER by create_time desc';
            break;
        case 'timestamp':
            $sql .= ' ORDER by timestamp desc';
            break;
        case 'sufferers':
            $sql .= ' ORDER by sufferers desc';
            break;
        case 'activity':
            $sql .= ' ORDER by activity desc';
            break;
        case 'score':
            $sql .= ' ORDER by score desc';
            break;
    }
    if ($min > -1) {
        $sql .= ' LIMIT '.$min;
        if ($nRec > -1) {
            $sql .= ', '.$nRec;
        }
    } else if ($nRec > -1) {
        $sql .= ' LIMIT '.$nRec;
    }

    return mysql_query($sql);
}

function getPosts($threadID, $min = -1, $nRec = -1, $sort_style="timestamp") {
    $sql = 'SELECT * FROM post WHERE thread = '. $threadID;
    switch($sort_style) {
    case 'timestamp':
        $sql = $sql . ' ORDER BY timestamp desc';
        break;
    case 'timestamp_asc':
        $sql = $sql . ' ORDER BY timestamp asc';
        break;
    case 'score':
        $sql = $sql . ' ORDER BY score DESC';
        break;
    }

    if ($min > -1) {
        $sql .= ' LIMIT '.$min;
        if ($nRec > -1) {
            $sql .= ', '.$nRec;
        }
    } elseif ($nRec > -1) {
        $sql .= ' LIMIT '.$nRec;
    }
    return mysql_query($sql);
}

/* specific database functions */

function getCategory($categoryID) {
    $sql = "SELECT * FROM category WHERE id = ".$categoryID;
    $result = mysql_query($sql);
    if ($result) {
      return mysql_fetch_object($result);
    } else {
      return NULL;
    }
}

function getForum($forumID) {
    $sql = "SELECT * FROM forum WHERE id = " . $forumID;
    $result = mysql_query($sql);
    if ($result) {
        return mysql_fetch_object($result);
    } else {
        return NULL;
    }
}

function getThread($threadID) {
    $sql = "SELECT * FROM thread WHERE id = ".$threadID;
    $result = mysql_query($sql);
    if ($result) {
        return mysql_fetch_object($result);
    } else {
        return NULL;
    }
}

function getPost($postID) {
    $sql = "SELECT * FROM post WHERE id = ".$postID;
    $result = mysql_query($sql);
    if ($result) {
        return mysql_fetch_object($result);
    } else {
        return NULL;
    }
}

// Returns the post that started the thread with id = $threadId
function getFirstPost($threadID) {
    $sql = "SELECT * FROM post WHERE thread = " . $threadID ." ORDER BY id ASC limit 1";
    $result = mysql_query($sql);
    if ($result) {
        return mysql_fetch_object($result);
    } else {
        return NULL;
    }
}

function incThreadViews($threadID) {
    $sql = "UPDATE thread SET views = views + 1 WHERE id = " . $threadID . " LIMIT 1";
    mysql_query($sql);
}

/* Forum modifying functions. */

function createThread($forumID, $ownerID, $title, $content) {

    $title = addslashes(sanitize_html($title));
    $content = addslashes(sanitize_html($content));

    $sql = "INSERT INTO thread (forum, owner, title, create_time, timestamp) VALUES (" . $forumID . ", " . $ownerID . ", '" . $title . "', UNIX_TIMESTAMP(), UNIX_TIMESTAMP())";
    $result = mysql_query($sql);
    if (!$result) return false;
    $threadID = mysql_insert_id();

    $postID = addPost($threadID, $ownerID, NULL, $content);

    $sql = "UPDATE user SET posts = posts + 1 WHERE id = " . $ownerID . " LIMIT 1";
    mysql_query($sql);

    $sql = "UPDATE forum SET threads = threads + 1, posts = posts + 1, timestamp = UNIX_TIMESTAMP() WHERE id = " . $forumID . " LIMIT 1";
    mysql_query($sql);

    return $threadID;
}

function replyToThread($threadID, $userID, $content, $parent_post=NULL) {
    $thread = getThread($threadID);

    $content = addslashes(sanitize_html(stripslashes($content)));

    addPost($threadID, $userID, $parent_post, $content);

    $sql = "UPDATE user SET posts = posts + 1 WHERE id = " . $userID . " LIMIT 1";
    mysql_query($sql);

    $sql = "UPDATE thread SET replies = replies + 1, timestamp = UNIX_TIMESTAMP() WHERE id = " . $threadID . " LIMIT 1";
    mysql_query($sql);

    $sql = "UPDATE forum SET posts = posts + 1, timestamp = UNIX_TIMESTAMP() WHERE id = " . $thread->forum . " LIMIT 1";
    mysql_query($sql);
}

function addPost($threadID, $userID, $parentID, $content) {
    if ($parentID) {
        $sql = "INSERT INTO post (thread, user, timestamp, content, parent_post) VALUES (" . $threadID . ", " . $userID . ", UNIX_TIMESTAMP(), '" . $content . "', " . $parentID . ")";
    } else {
        $sql = "INSERT INTO post (thread, user, timestamp, content) VALUES (" . $threadID . ", " . $userID . ", UNIX_TIMESTAMP(), '" . $content . "')";
    }
    $result = mysql_query($sql);
    if (!$result) return false;
    //return ($post->id = mysql_insert_id());
    mysql_insert_id();
    return true;
}

function updatePost($postID, $content) {
    $x = addslashes(sanitize_html(stripslashes($content)));
    $sql = "UPDATE post SET content = \"$x\", modified = UNIX_TIMESTAMP() WHERE id = " . $postID;
    $result = mysql_query($sql);
    if (!$result) return false;
    return true;
}

/* display functions */

function show_posts($thread, $sort_style, $filter, $show_controls=true, $do_coloring=true, $is_helpdesk=false) {
    global $logged_in_user;
    $n = 1;

    if ($show_controls && !$is_helpdesk) {
        $controls = FORUM_CONTROLS;
    } else if ($show_controls && $is_helpdesk) {
        $controls = HELPDESK_CONTROLS;
    } else {
        $controls = NO_CONTROLS;
    }

    $posts = getPosts($thread->id, -1, -1, $sort_style);
    $firstPost = getFirstPost($thread->id);

    if ($is_helpdesk) {
        if ($firstPost) {
            show_post($firstPost, $thread, $logged_in_user, $n, $controls, true);
        }
    }

    while ($post = mysql_fetch_object($posts)) {
        if ($post->score >= $filter) {
            if (!$is_helpdesk || ($is_helpdesk && $post->id != $firstPost->id)) {
                show_post($post, $thread, $logged_in_user, $n, $controls, false);
                if ($do_coloring) $n = ($n+1)%2;
            }
        }
    }
}

function show_post($post, $thread, $logged_in_user, $n, $controls=FORUM_CONTROLS, $separate=false) {
    global $post_ratings;

    $user = lookup_user_id($post->user);
    $sql = "SELECT * FROM profile WHERE userid = " . $user->id;
    $result2 = mysql_query($sql);
    $user->has_profile = (mysql_num_rows($result2) > 0);

    $can_edit = $logged_in_user && $user->id == $logged_in_user->id;

    echo "
        <tr class=\"row$n\" style=\"vertical-align:top\">
        <td>
        <a name= $post->id ></a>
        <p style=\"font-weight:bold\">
    ";

    echo user_links($user, "../");

    echo "
        </p>
        <p style=\"font-size:8pt\">
        Joined: ", gmdate('M j, Y', $user->create_time), "<br>Posts: ", $user->posts, "
        </p>
        </td>
        <td>
    ";

    if ($controls == FORUM_CONTROLS || $controls == HELPDESK_CONTROLS) {
        echo "<form action=\"rate.php?post=", $post->id, "\" method=\"post\">";
    }
    echo "
        <table width=100% cellspacing=0 cellpadding=0>
        <tr valign=\"top\">
        <td align=\"left\" style=\"border:0px\"><p style=\"font-size:8pt\">
        Posted: ", pretty_time_str($post->timestamp);
    ;

    if ($post->parent_post) echo " in response to <a href=#$post->parent_post>Message ID $post->parent_post</a>.";
    if ($can_edit && $controls != NO_CONTROLS) echo "&nbsp;<a href=\"edit.php?id=$post->id\">[Edit this post]</a>";
    if ($post->modified) echo "<br>Last modified: ", pretty_time_Str($post->modified);

    echo "</p>\n</td>\n";

    if ($controls == FORUM_CONTROLS) {
        //echo "<td align=\"right\" style=\"border:0px\">Rate this post:";
        //show_select_from_array("rating", $post_ratings, "0");
        //echo "<input type=\"submit\" value=\"Rate\"></td>";
    } else if ($controls == HELPDESK_CONTROLS && $separate) {
        echo "
            <td align=\"right\" style=\"border:0px\">
            <input type=\"submit\" name=\"submit\" value=\"", SUFFERER, "\">
            </td>
        ";
    } else if ($controls == HELPDESK_CONTROLS && !$separate) {
        echo "
            <td align=\"right\" style=\"border:0px\">
            <input type=\"submit\" name=\"submit\" value=\"", SOLUTION, "\">
            <input type=\"submit\" name=\"submit\" value=\"", OFF_TOPIC, "\">
            </td>
        ";
    }

    echo "</tr>\n</table>\n";

    if ($controls == FORUM_CONTROLS || $controls == HELPDESK_CONTROLS) {
        echo "</form>";
    }
    echo "<p>", nl2br(stripslashes($post->content)), "</p>";
    echo "<table width=100% cellspacing=0 cellpadding=0>
        <tr valign=\"bottom\">
        <td align=\"left\" style=\"border:0px; font-size:7pt\"><i>ID: ", $post->id;
    if ($controls == HELPDESK_CONTROLS && $separate) {
        echo "</i></td>";
    } else if ($controls == HELPDESK_CONTROLS && !$separate) {
        echo " / Score: ", ($post->score * $post->votes), "</i></td>";
    } else {
        echo " / Rating: ", $post->score, "</i></td>";
    }

    if ($controls == FORUM_CONTROLS) {
        echo "<td align=\"right\" style=\"border:0px\">[<a href=\"reply.php?thread=" . $thread->id . "&post=" . $post->id . "#input\">Reply to this post</a>]</td>";
    } else if ($controls == HELPDESK_CONTROLS && !$separate) {
        echo "<td align=\"right\" style=\"border:0px\">[<a href=\"reply.php?thread=" . $thread->id . "&post=" . $post->id . "&helpdesk=1#input\">Reply to this answer</a>]</td>";
    }

    echo "</tr></table></td></tr>";

    if ($separate) {
        echo "
            </table>
            <br><br>
            <table class=\"content\" border=0 cellpadding=5 cellspacing=0 width=100%>
            <tr>
            <th style=\"width: 150px\">Author</th>
            <th>Answers</th>
        ";
    }
}

/* utility functions */

function start_forum_table($headings, $span=NULL) {
    echo "
        <p style=\"text-align:center\">
        <table order=0 cellpadding=5 cellspacing=0 width=100%>
        <tr>
    ";

    for ($i = 0; $i < count($headings); $i++) {
        $cell = "<th";
        if ($span) {
            $cell = $cell . " colspan=$span";
        }
        $cell = $cell . ">";
        echo $cell, $headings[$i], "</th>\n";
    }
    echo "</tr>\n";
}

function end_forum_table() {
    echo "</table></p>\n";
}

// generate a "select" element from an array of values
//
function show_select_from_array($name, $array, $selection) {
    echo "<select name=\"$name\">";

    foreach ($array as $key => $value) {
        echo "<option ";
        if ($key == $selection) {
            echo "selected ";
        }
        echo "value=\"", $key, "\">", $value, "</option>";
    }
    echo "</select>";
}

function show_forum_title($forum=NULL, $thread=NULL, $helpdesk=false) {
    echo "<p>\n";
    if (!$forum && !$thread) {
        echo "<p class=\"title\">";
        if ($helpdesk) {
            echo " Questions and problems</p>";
        } else {
            echo " Message boards</p>";
        }
    } else if ($forum && !$thread) {
        echo "<span class=title>";
        if ($helpdesk) {
            echo "<a href=help_desk.php>", " Questions and problems</a> : ";
        } else {
            echo "<a href=index.php>", " Message boards</a> : ";
        }
        echo $forum->title;
        echo "</span><br>";
    } else if ($forum && $thread) {
        echo "<span class=title>";
        if ($helpdesk) {
            echo "<a href=help_desk.php>", " Questions and problems</a> : ";
        } else {
            echo "<a href=index.php>", " Message boards</a> : ";
        }
        echo "<a href=\"forum.php?id=", $forum->id, "\">", $forum->title, "</a> : ";
        echo stripslashes($thread->title);
        echo "</span><br>";
    } else {
        echo "Invalid input to show_forum_title<br>";
    }
    echo "</p>\n";
}

// show a thread with its context (e.g. for search results)
//
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

// show a post with its context (e.g. for search results)
//
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

?>
