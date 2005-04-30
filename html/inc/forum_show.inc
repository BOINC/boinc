<?php
// Number of forum topics per page.
$threads_per_page = 50;

// print links to other pages in this forum
// If there are more than the threshold number of threads on the page,
// show only the first $thread_per_page and display links to the rest
//
//
function show_page_nav($forum) {
    global $threads_per_page;

    if ($forum->threads > $threads_per_page) {
        $totalPages = ceil($forum->threads / $threads_per_page);
        $curPage = floor($_GET['start'] / $threads_per_page);

        if ($totalPages == 2) {
            $pages = array(0, 1);
        } else {
            $pages = array(0, 1, 2);
        }
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
            $gotoStr .= '1 ';
        } else {
            $gotoStr .= '<a href="forum_forum.php?id='.$_GET['id'];
            $gotoStr .= '&start='.(($curPage-1)*$threads_per_page);
            $gotoStr .= '&sort='.$_GET["sort"];
            $gotoStr .= '">Previous</a> <a href="forum_forum.php?id='.$_GET['id'].'&start=0">1</a>';
        }

        for ($i = 1 ; $i < count($pages)-1 ; $i++) {
            if ($curPage == $pages[$i]) {
                $gotoStr .= ($i > 0 && $pages[$i-1] == $pages[$i] - 1)?', ':' ... ';
                $gotoStr .= $pages[$i]+1;
            } else {
                $gotoStr .= ($i > 0 && $pages[$i-1] == $pages[$i] - 1)?', ':' ... ';
                $gotoStr .= '<a href="forum_forum.php?id='.$_GET['id'];
                $gotoStr .= '&start='.($pages[$i]*$threads_per_page);
                $gotoStr .= '&sort='.$_GET["sort"];
                $gotoStr .= '">'.($pages[$i]+1).'</a>';
            }
        }

        if ($curPage == $totalPages-1) {
            $gotoStr .= $totalPages;
        } else {
            $gotoStr .= ', <a href="forum_forum.php?id='.$_GET['id'];
            $gotoStr .= '&start='.(($totalPages-1)*$threads_per_page);
            $gotoStr .= '&sort='.$_GET["sort"];
            $gotoStr .= '">'.$totalPages.'</a> ';
            $gotoStr .= '<a href="forum_forum.php?id='.$_GET['id'];
            $gotoStr .= '&start='.(($curPage+1)*$threads_per_page);
            $gotoStr .= '&sort='.$_GET["sort"];
            $gotoStr .= '">Next</a>';
        }

        $gotoStr .= '</p>';

        echo $gotoStr;
    }
    return $gotoStr;
}

function show_forum($category, $forum, $start, $sort_style, $logged_in_user) {
    global $threads_per_page;
    $gotoStr = show_page_nav($forum);
    if ($category->is_helpdesk) {
        start_forum_table(array("Question", "Answers"));
    } else {
        start_forum_table(array("", "Threads", "Posts", "Author", "Views", "Last post"));
    }
    $sticky_first = !$logged_in_user->ignore_sticky_posts;
    $threads = getThreads($forum->id, $start, $threads_per_page, $sort_style, 0, $sticky_first);
    $n = 0;

    while ($thread = mysql_fetch_object($threads)) {
        $user = lookup_user_id($thread->owner);
        $logged_in_user=getThreadLastVisited($logged_in_user,$thread);
        $unread = ($thread->timestamp>$logged_in_user->thread_last_visited);

        echo "
            <tr class=row$n style=\"text-align:center\">";

        if (!$category->is_helpdesk) {		        //Show thread icons if in forum:
            echo "<td width=\"1%\" align=\"right\"><nobr>";
            $first_post = getFirstPost($thread->id);
            if ($first_post->score*$first_post->votes>$logged_in_user->high_rating_threshold) {
                echo "<img src=\"".EMPHASIZE_IMAGE."\" alt=\"Emphasized thread\">";
            }
            if ($first_post->score*$first_post->votes<$logged_in_user->low_rating_threshold) {
                echo "<img src=\"".FILTER_IMAGE."\" alt=\"Filtered thread\">";
            }
            if ($unread && !$thread->sticky) {
                echo "<img src=\"".NEW_IMAGE."\" alt=\"Unread post(s)\">";
            }
	    elseif($unread) {
                echo "<img src=\"".NEW_IMAGE_STICKY."\" alt=\"Unread post(s)\">";
            }
	    elseif($thread->sticky) {
                echo "<img src=\"".STICKY_IMAGE."\" alt=\"Sticky\">";
            }
            echo "</nobr></td>";
        }
        echo "<td style=\"font-size:10pt; text-align:left\"><a href=\"forum_thread.php?id=", $thread->id, "\"><b>", cleanup_title($thread->title), "</b></a><br>";
        $n = ($n+1)%2;

        if ($category->is_helpdesk) {
            $first_post = getFirstPost($thread->id);
            $excerpt = sub_sentence($first_post->content, ' ', EXCERPT_LENGTH, true);
            echo strip_tags(stripslashes($excerpt));
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
                <td align=left>", user_links($user), "</td>
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
}

?>
