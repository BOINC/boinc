<?php

require_once('forum.inc');
require_once('../util.inc');
require_once('../time.inc');

// Number of forum topics per page.
// TODO: Make this a constant.
$n = 50;

if (empty($_GET['id'])) {
    // TODO: Standard error page
    echo "Invalid forum ID.<br>";
    exit();
}

$_GET['id'] = stripslashes(strip_tags($_GET['id']));
$_GET['sort'] = stripslashes(strip_tags($_GET['sort']));

if (!array_key_exists('start', $_GET) || $_GET['start'] < 0) {
    $_GET['start'] = 0;
}

$forum = getForum($_GET['id']);
$category = getCategory($forum->category);

if ($category->is_helpdesk) {
    $sort_style = $_GET['sort'];
    if (!$sort_style) {
        $sort_style = $_COOKIE['hd_sort_style'];
    } else {
        setcookie('hd_sort_style', $sort_style, time()+3600*24*365);
    }
    if (!$sort_style) $sort_style = 'activity';
    page_head('Help Desk');
} else {
    $sort_style = $_GET['sort'];
    if (!$sort_style) {
        $sort_style = $_COOKIE['forum_sort_style'];
    } else {
        setcookie('forum_sort_style', $sort_style, time()+3600*24*365);
    }
    if (!$sort_style) $sort_style = 'modified-new';
    page_head('Message boards : '.$forum->title);
}

echo "
    <form action=\"forum.php\" method=\"get\">
    <input type=\"hidden\" name=\"id\" value=", $forum->id, ">
    <table width=100% cellspacing=0 cellpadding=0>
    <tr valign=\"bottom\">
    <td align=\"left\" style=\"border:0px\">
";

show_forum_title($forum, NULL, $category->is_helpdesk);

echo "<p>\n<a href=\"post.php?id=", $_GET['id'], "\">";

if ($category->is_helpdesk) {
    echo "Submit a question or problem";
} else  {
    echo "Create a new thread";
}

echo "</a>\n</p>\n</td>";

echo "<td align=right>";
if ($category->is_helpdesk) {
    show_select_from_array("sort", $faq_sort_styles, $sort_style);
} else {
    show_select_from_array("sort", $forum_sort_styles, $sort_style);
}
echo "<input type=submit value=OK></td>\n";

echo "</tr>\n</table>\n</form>";

// If there are more than the threshold number of threads on the page,
// show only the first $n and display links to the rest
//
show_page_nav($forum);

if ($category->is_helpdesk) {
    start_forum_table(array("Question", "Answers"));
} else {
    start_forum_table(array("Threads", "Posts", "Author", "Views", "Last post"));
}

// TODO: Move this into its own function?

$threads = getThreads($forum->id, $_GET['start'], $n, $sort_style);
$n = 0;

while($thread = mysql_fetch_object($threads)) {
    $user = lookup_user_id($thread->owner);
    $first_post = getFirstPost($thread->id);
    $excerpt = sub_sentence($first_post->content, ' ', EXCERPT_LENGTH, true);
    echo "
        <tr class=row$n style=\"font-size:8pt; text-align:center\">
        <td style=\"font-size:10pt; text-align:left\"><a href=\"thread.php?id=", $thread->id, "\"><b>", stripslashes($thread->title), "</b></a><br>
    ";
    $n = ($n+1)%2;

    if ($category->is_helpdesk) {
        echo stripslashes($excerpt);
        $na = $thread->sufferers + 1;
        $x = time_diff_str($first_post->timestamp, time());
        echo "<br><font size=-2>Asked $x; asked $na times";
    }

    echo "</td>";
    $x = time_diff_str($thread->timestamp, time());
    if ($category->is_helpdesk) {
        if ($thread->replies == 0) $x = "---";
        echo "<td align=left>
            Total: $thread->replies
            <br>Last: $x
            </td>
        ";
    } else {
        echo "
            <td>", $thread->replies+1, "</td>
            <td align=left>", user_links($user, "../"), "</td>
            <td>", $thread->views, "</td>
            <td style=\"text-align:right\">", $x, "</td>
        ";
    }

    echo "</tr>";
}

end_forum_table();

if ($forum->threads > $n) {
    echo $gotoStr;
}

page_tail();


function show_page_nav($forum) {
    global $n;

    if ($forum->threads > $n) {
        $totalPages = floor($forum->threads / $n);
        $curPage = floor($_GET['start'] / $n);

        $pages = array(0, 1, 2);
        for ($i = -1 ; $i <= 1 ; $i++) {
            if ($curPage + $i > 0 && $curPage + $i < $totalPages - 1) {
                array_push($pages, $curPage + $i);
            }
        }
        for ($i = -3 ; $i <= -1 ; $i++) {
            if ($totalPages + $i > 0) {
                array_push($pages, $totalPages + $i);
            }
        }
        $pages = array_unique($pages);
        natsort($pages);
        $pages = array_values($pages);

        $gotoStr = '<p style="text-align:right">Go to page ';

        if ($curPage == 0) {
            $gotoStr .= '<span style="font-size:larger; font-weight:bold">1</span>';
        } else {
            $gotoStr .= '<a href="forum.php?id='.$_GET['id'];
            $gotoStr .= '&start='.(($curPage-1)*$n);
            $gotoStr .= '&sort='.$_GET["sort"];
            $gotoStr .= '">Previous</a> <a href="forum.php?id='.$_GET['id'].'&start=0">1</a>';
        }

        for ($i = 1 ; $i < count($pages)-1 ; $i++) {
            if ($curPage == $pages[$i]) {
                $gotoStr .= ($i > 0 && $pages[$i-1] == $pages[$i] - 1)?', ':' ... ';
                $gotoStr .= '<span style="font-size:larger; font-weight:bold">'.($pages[$i]+1).'</span>';
            } else {
                $gotoStr .= ($i > 0 && $pages[$i-1] == $pages[$i] - 1)?', ':' ... ';
                $gotoStr .= '<a href="forum.php?id='.$_GET['id'];
                $gotoStr .= '&start='.($pages[$i]*$n);
                $gotoStr .= '&sort='.$_GET["sort"];
                $gotoStr .= '">'.($pages[$i]+1).'</a>';
            }
        }

        if ($curPage == $totalPages-1) {
            $gotoStr .= ', <span style="font-size:larger; font-weight:bold">'.$totalPages.'</span>';
        } else {
            $gotoStr .= ', <a href="forum.php?id='.$_GET['id'];
            $gotoStr .= '&start='.(($totalPages-1)*$n);
            $gotoStr .= '&sort='.$_GET["sort"];
            $gotoStr .= '">'.$totalPages.'</a> ';
            $gotoStr .= '<a href="forum.php?id='.$_GET['id'];
            $gotoStr .= '&start='.(($curPage+1)*$n);
            $gotoStr .= '&sort='.$_GET["sort"];
            $gotoStr .= '">Next</a>';
        }

        $gotoStr .= '</p>';

        echo $gotoStr;
    }
}
?>
