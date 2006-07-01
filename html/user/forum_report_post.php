<?php
/**
 * Some posts may contain material that is not suited for public
 * viewing. This file allows people to report such posts
 * For this file to work the project must have defined who
 * should receive such reports (in the configuration file)
 **/

require_once('../inc/forum.inc');
require_once('../inc/forum_email.inc');
require_once('../inc/forum_std.inc');

db_init();

    $postId = get_int('post');

    $post = new Post($postId);
    $thread = $post->getThread();
    $forum = $thread->getForum();

    $user = re_get_logged_in_user(true);
    
    /* Make sure the user has the forum's minimum amount of RAC and total credit
     * before allowing them to report a post. Using the same rules as for rating (at least for now)
     */
    if ($user->getTotalCredit()<$forum->getRateMinTotalCredit() || $user->getExpavgCredit()<$forum->getRateMinExpavgCredit()) {
        error_page("You need more average or total credit to report a post.");
    }
    
    if ($post->hasReported($user)) {
        error_page("You have already reported this post once.");
    }

//__-------------- Action part
    $success_page=0;
    if (get_str("submit",true)){
	$reason = get_str("reason");
	if (send_report_post_email($user, $thread, $post, $reason)){
	    $success_page=1;
	} else {
	    $success_page=-1;
	}
    }



//__--------------- Display part
    if ($success_page==1) {
        $post_thread = $post->getThread();
        page_head('Report Registered');
        echo "<p>Your report has been successfully recorded.  Thank you for your input.</p>
	    <p>A moderator will now look at your report and decide what will happen - this may take a little while, so please be patient</p>";
        echo "<a href=\"forum_thread.php?id=", $post_thread->getID(), "#", $post->getID(), "\">Return to thread</a>";
    } elseif($success_page==0){
        page_head('Report a forum post'); 
	echo "<p>Before reporting this post <em>please</em> consider using the +/- rating system instead. If enough users agree on rating a post negatively it will
	    eventually be hidden. <br />You can find the rating system at the bottom of the post.</p>";
	start_forum_table(array(tr(FORUM_AUTHOR), tr(FORUM_MESSAGE),""));
	    show_post($post,$thread, $user,0);
	    echo "<form action=\"forum_report_post.php\" method=\"get\">\n";
	    row1("Report post");
	    row2("Why do you find the post offensive:<br><font size=-1>Please include enough information so that a person that 
	    has not yet read the thread will quickly be able to identify the issue.</font>",
	    "<textarea name=\"reason\" rows=12 cols=54></textarea>");
	    row2("",
	    "<input type=\"submit\" name=\"submit\" value=\"OK\">");
	    echo "<input type=\"hidden\" name=\"post\" value=\"".$post->getID()."\">";
	    echo "</form>";		    
	end_table();
    } elseif ($success_page==-1) {
        page_head('Report NOT registered');
        echo "<p>Your report could not be recorded.  Please wait a short while and try again.</p>
	    <p>If this is not a temporary error, please report it to the project developers.</p>";
        echo "<a href=\"forum_thread.php?id=", $post->getThread(), "#", $post->getID(), "\">Return to thread</a>";
    }
    page_tail();
?>
