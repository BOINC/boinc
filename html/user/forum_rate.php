<?php
/**
 * This file allows people to rate posts in a thread
 **/
require_once('../inc/forum.inc');
require_once('../inc/forum_std.inc');
require_once('../inc/util.inc');

$config = get_config();
if (parse_element($config,"<no_forum_rating>")) {
        page_head("Rating offline");
        echo "This function is turned off by the project";
        page_tail();
        exit(0);
        }

db_init();

if (!empty($_GET['post'])) {
    $postId = get_int('post');
    $choice = post_str('submit', true);
    $rating = post_int('rating', true);
    if (!$choice) $choice = get_str('choice', true);
    
    if ($choice == SOLUTION or $choice=="p") {
        $rating = 1;
    } else {
        $rating = -1;
    }


    $user = re_get_logged_in_user(true);

    if ($choice == null && ($rating == null || $rating > 2 || $rating < -2)) {
        show_result_page(false, NULL, $choice);
    }

    $post = new Post($postId);
    $thread = $post->getThread();
    $forum = $thread->getForum();


    /* Make sure the user has the forum's minimum amount of RAC and total credit
     * before allowing them to rate a post.
     */
    if ($user->getTotalCredit()<$forum->getRateMinTotalCredit() || $user->getExpavgCredit()<$forum->getRateMinExpavgCredit()) {
        error_page("You need more average or total credit to rate a post.");
    }
    
    if ($post->hasRated($user)) {
        $post_thread = $post->getThread();
        error_page("You have already rated this post once.<br /><br /><a href=\"forum_thread.php?nowrap=true&id=".$post_thread->getID()."#".$post->getID()."\">Return to thread</a>");
    } else {
        $success = $post->rate($user, $rating);
        show_result_page($success, $post, $choice);
    }
}

function show_result_page($success, $post, $choice) {
    if ($success) {
        if ($choice) {
            page_head('Input Recorded');
                echo "<p>Your input has been successfully recorded.  Thank you for your help.</p>";
        } else {
            page_head('Vote Registered');
        echo "<span class=\"title\">Vote Registered</span>";
        echo "<p>Your rating has been successfully recorded.  Thank you for your input.</p>";
        }
        $post_thread = $post->getThread();
        echo "<a href=\"forum_thread.php?nowrap=true&id=", $post_thread->getID(), "#", $post->getID(), "\">Return to thread</a>";
    } else {
        page_head('Vote Submission Problem');    
        echo "<span class=\"title\">Vote submission failed</span>";
        if ($post) {
            echo "<p>There was a problem recording your vote in our database.  Please try again later.</p>";
            $post_thread = $post->getThread();
            echo "<a href=\"forum_thread.php?id=", $post_thread->getID(), "#", $post->getID(), "\">Return to thread</a>";
        } else {
            echo "<p>There post you specified does not exist, or your rating was invalid.</p>";
        }
    }
    page_tail();
    exit;
}
?>
