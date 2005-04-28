<?php

require_once('../inc/forum.inc');
require_once('../inc/util.inc');
require_once('../inc/credit.inc');


db_init();

if (!empty($_GET['post'])) {
    $postId = get_int('post');
    $choice = post_str('submit', true);
    $rating = post_int('rating', true);
    if (!$choice) $choice = get_str('choice', true);
    
    if ($choice == SOLUTION or $choice=="p") {
        $rating = 1;
    } else if ($choice == OFF_TOPIC or $choice=="n") {
        $rating = -1;
    }

    // TODO: Define constants for these.
    if ($choice == null && ($rating == null || $rating > 2 || $rating < -2)) {
        show_result_page(false, NULL, $choice);
        exit();
    }

    $post = getPost($postId);
    $thread = getThread($post->thread);
    $forum = getForum($thread->forum);

    $user = get_logged_in_user(true);
    $user = getForumPreferences($user);

    /* Make sure the user has the forum's minimum amount of RAC and total credit
     * before allowing them to rate a post.
     */
    if ($user->total_credit<$forum->rate_min_total_credit || $user->expavg_credit<$forum->rate_min_expavg_credit) {
        error_page("You need more average or total credit to rate a post.");
    }
    
    if (getHasRated($user,$postId)) {
        error_page("You have already rated this post once.");
    } else {
        $result = mysql_query("SELECT * FROM post WHERE id = $postId");
        if ($result) {
            if (mysql_num_rows($result) > 0) {
                $post = mysql_fetch_object($result);

                if ($choice == NULL || $choice == SOLUTION || $choice == OFF_TOPIC || $choice=="p" || $choice=="n") {
                    $points = $post->votes * $post->score;
                    $votes = $post->votes + 1;
                    $score = ($points + $rating) / $votes;

                    $result2 = mysql_query("UPDATE post SET votes = $votes, score = $score WHERE id = $postId");
                } else if ($choice == SUFFERER) {
                    $sql = "UPDATE thread SET sufferers = sufferers + 1 WHERE id = " . $post->thread;
                    $result2 = mysql_query($sql);
                }

                if ($result2) {
                    show_result_page(true, $post, $choice);
                    setHasRated($user,$postId);
                } else {
                    show_result_page(false, $post, $choice);
                }
            } else {
                show_result_page(false, NULL, $choice);
            }
        } else {
            show_result_page(false, NULL, $choice);
        }
    }
}

function show_result_page($success, $post, $choice) {
    $logged_in_user = get_logged_in_user(false);

    if ($success) {
        if ($choice) {
            page_head('Input Recorded');
                echo "<p>Your input has been successfully recorded.  Thank you for your help.</p>";
        } else {
            page_head('Vote Registered');
        echo "<span class=\"title\">Vote Registered</span>";
        echo "<p>Your rating has been successfully recorded.  Thank you for your input.</p>";
        }
        echo "<a href=\"forum_thread.php?id=", $post->thread, "#", $post->id, "\">Return to thread</a>";
    } else {
        page_head('Vote Submission Problem');    
        echo "<span class=\"title\">Vote submission failed</span>";
        if ($post) {
            echo "<p>There was a problem recording your vote in our database.  Please try again later.</p>";
            echo "<a href=\"forum_thread.php?id=", $post->thread, "#", $post->id, "\">Return to thread</a>";
        } else {
            echo "<p>There post you specified does not exist, or your rating was invalid.</p>";
        }
    }
    page_tail();
}
?>
