<?php

require_once('../include/template.inc');
require_once('forum.inc');
require_once('../util.inc');

if (!empty($_GET['post'])) {
    $postId = $_GET['post'];
    $choice = $_POST['submit'];
    $rating = $_POST['rating'];

    if ($choice == SOLUTION) {
        $rating = 1;
    } else if ($choice == OFF_TOPIC) {
        $rating = -1;   
    }
    
    // TODO: Define constants for these.
    if ($choice == null && ($rating == null || $rating > 2 || $rating < -2)) {
        show_result_page(false, NULL, $choice);
        exit();
    }
    
    $user = get_logged_in_user($true, '../');   

    $result = mysql_query("SELECT * FROM post WHERE id = $postId");
   	if ($result) {
        if (mysql_num_rows($result) > 0) {
   	        $post = mysql_fetch_object($result);
   	        
   	        if ($choice == NULL || $choice == SOLUTION || $choice == OFF_TOPIC) {
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

function show_result_page($success, $post, $choice) {
    if ($success) {
	    if ($choice) {
		 	doHeader('Input Recorded');
			echo "<span class=\"title\">Helpdesk Input Recorded</span>";
			echo "<p>Your input has been successfully recorded.  Thank you for your help.</p>";
	    } else {
        	doHeader('Vote Registered');
        	echo "<span class=\"title\">Vote Registered</span>";
        	echo "<p>Your rating has been successfully recorded.  Thank you for your input.</p>";
    	}
    	echo "<a href=\"thread.php?id=", $post->thread, "#", $post->id, "\">Return to thread</a>";
    } else {
        doHeader("Vote Submission Problem");
        echo "<span class=\"title\">Vote submission failed</span>";
        if ($post) {
            echo "<p>There was a problem recording your vote in our database.  Please try again later.</p>";
            echo "<a href=\"thread.php?id=", $post->thread, "#", $post->id, "\">Return to thread</a>";
        } else {
            echo "<p>There post you specified does not exist, or your rating was invalid.</p>";
        }
    }
    doFooter();
}
?>