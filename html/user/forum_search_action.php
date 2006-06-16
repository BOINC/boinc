<?php
/**
 * This file allows people to search for posts or a thread.  Takes input from 
 * forum_search.php
 **/
 
require_once('../inc/forum.inc');
require_once('../inc/forum_std.inc');


db_init();
$logged_in_user = re_get_logged_in_user(true);

page_head(tr(FORUM_SEARCH));

$search_keywords = post_str("search_keywords", true);
$search_author = post_str("search_author", true);
$search_max_time = post_int("search_max_time");
$search_forum = post_int("search_forum");
$search_sort = post_int("search_sort");
$search_list = explode(" ",$search_keywords);
$min_timestamp = time() - ($search_max_time*3600*24);
$limit = 100;


if ($search_forum==-1){
    $forum = "";
} else if ($search_forum) {
    $forum = new Forum($search_forum);
}
if ($search_author) $user = newUser($search_author);

$dbhandler = $mainFactory->getDatabaseHandler();

// First search thread titles, if we get a hit there it's almost bound to be relevant
$thread_ids = $dbhandler->searchThreadTitles($search_list, $forum, $user, $min_timestamp, round($limit/7), $search_sort);

// Display the threads while we search for posts
//STUB todo
if ($thread_ids){
    echo "<p><a href=\"forum_search.php\">Perform another search</a></p>";
    echo "<h2>Threads found matching your search query:</h2>";
    start_forum_table(array(tr(FORUM_TOPIC), tr(FORUM_THREADS), tr(FORUM_POSTS), tr(FORUM_AUTHOR), tr(FORUM_VIEWS), "<nobr>".tr(FORUM_LAST_POST)."</nobr>"));
    foreach ($thread_ids as $key => $thread_id){
	$thread = new Thread($thread_id);
	if ($thread->isHidden()) continue;
    $thread_forum = $thread->getForum();
	echo '
	<tr>
	    <td>'.$thread_forum->getTitle().'</td>
	    <td class="threadline"><a href="forum_thread.php?id='.$thread->getID().'"><b>'.$thread->getTitle().'</b></a></td>';
	echo '
    	    <td>'.($thread->getPostCount()+1).'</td>
    	    <td align="left"><div class="authorcol">'.re_user_links($thread->getOwner()).'</div></td>
    	    <td>'.$thread->getViewCount().'</td>
    	    <td style="text-align:right">'.time_diff_str($thread->getLastTimestamp(), time()).'</td>
	</tr>';
    }
    end_table();
    echo "<br /><br />";
}

// Let's see if we can match anything in a post body as well:
$post_ids = $dbhandler->searchPosts($search_list, $forum, $user, $min_timestamp, $limit, $search_sort);

if ($post_ids){
    echo "<h2>Posts found matching your search query:</h2>";
    start_forum_table(array(tr(FORUM_TOPIC), tr(FORUM_THREADS), tr(FORUM_AUTHOR),"<nobr>".tr(FORUM_LAST_POST)."</nobr>"));
    if ($logged_in_user){
        $options = $logged_in_user->getTextTransformSettings();
    } else {
        $options = new output_options;
    }
    foreach ($post_ids as $key => $post_id){
	$post = new Post($post_id);
	$thread = $post->getThread();
	if ($thread->isHidden()) continue;
	if ($post->isHidden()) continue;
	$options->setHighlightTerms($search_list);
	$contents = output_transform($post->getContent(),$options);
    $thread_forum = $thread->getForum();
	echo '
	<tr>
	    <th>'.$thread_forum->getTitle().'</th>
	    <th class="threadline"><a href="forum_thread.php?id='.$thread->getID().'"><b>'.$thread->getTitle().'</b></a></th>
    	    <th align="left"><div class="authorcol">'.re_user_links($post->getOwner()).'</div></th>
    	    <th style="text-align:right">'.time_diff_str($post->getTimestamp(), time()).'</th>
	</tr>
	<tr>
	    <td colspan=4>'.substr($contents,0,200).'...
	    </td>
	</tr>
	<tr><td colspan=4 class="postfooter">
    	    <a href="forum_thread.php?id='.$thread->getID().'&nowrap=true#'.$post->getID().'"><b>[Read the rest of this post]</b></a>
	</td></tr>
	<tr class="postseperator"><td colspan=4>&nbsp;</td></tr>
	';
    }
    end_table();
}

if (!$thread_ids && !$post_ids){
    echo "<p>Sorry, couldn't find anything matching your search query. You 
    can try to broaden your search by using less words (or less specific words).</p>
    <p>You can also 
    <a href=\"http://www.google.com/search?domains=".URL_BASE."&sitesearch=".URL_BASE."forum_thread.php&q=".htmlentities($search_keywords)."\">
    try the same search on Google.</a></p>";
}
echo "<p><a href=\"forum_search.php\">Perform another search</a></p>";
page_tail();
exit;

?>
