<?php
require_once("../util.inc");
require_once("../include.php");
require_once("forum.inc");

if (!empty($_GET['post']) && !empty($_POST['rating'])) {
    $postId = $_GET['post'];
    $rating = $_POST['rating'];
    
    $user = get_logged_in_user($true, '../');
    $result = mysql_query("SELECT * FROM post WHERE id = $postId");
    if ($result) {
        if (mysql_num_rows($result) > 0) {
            $post = mysql_fetch_object($result);
            $votes = $post->votes + 1;
            $score = $post->score + ($rating / $votes);
            
            $result2 = mysql_query("UPDATE post SET votes = $votes, score = $score WHERE id = $postId");
            if ($result2) {
                show_result_page(true, $post);
            } else {
                show_result_page(false, $post);
            }
        } else {
            show_result_page(false, NULL);
        }
    } else {
        show_result_page(false, NULL);
    }
}

function show_result_page($success, $post) {
    if ($success) {
        doHeader("Vote Registered");
        echo "<span class=\"title\">Vote Registered</span>";
        echo "<p>Your rating has been successfully recorded.  Thank you for your input.</p>";
    } else {
        doHeader("Vote Submission Problem");
        echo "<span class=\"title\">Vote submission failed</span>";
        if ($post) {
            echo "<p>There was a problem recording your vote in our database.  Please try again later.</p>";
        } else {
            echo "<p>There post you specified does not exist.</p>";
        }
    }
    doFooter();
}
?>