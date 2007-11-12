<?php
// search for posts or a thread.
// Takes input from forum_search.php
 
require_once('../inc/time.inc');
require_once('../inc/text_transform.inc');
require_once('../inc/forum.inc');

$logged_in_user = get_logged_in_user(false);
BoincForumPrefs::lookup($logged_in_user);
if ($logged_in_user && $logged_in_user->prefs->privilege(S_MODERATOR)){
    $show_hidden_posts = true;
} else {
    $show_hidden_posts = false;
}

page_head(tra("Forum search"));

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
    $forum = BoincForum::lookup_id($search_forum);
}

$user = null;
if ($search_author) {
    $user = BoincUser::lookup_id($search_author);
}

// First search thread titles, if we get a hit there it's probablyrelevant
//
$threads = search_thread_titles($search_list, $forum, $user, $min_timestamp, round($limit/7), $search_sort, $show_hidden_posts);

// Display the threads while we search for posts
if (count($threads)){
    echo "<p><a href=\"forum_search.php\">Perform another search</a></p>";
    echo "<h2>Threads found matching your search query:</h2>";
    start_forum_table(array(tra("Topic"), tra("Threads"), tra("Posts"), tra("Author"), tra("Views"), "<nobr>".tra("Last post")."</nobr>"));
    foreach ($threads as $thread){
        if ($thread->hidden) continue;
        $thread_forum = BoincForum::lookup_id($thread->forum);
        $owner = BoincUser::lookup_id($thread->owner);
        echo '
            <tr>
            <td>'.cleanup_title($thread_forum->title).'</td>
            <td class="threadline"><a href="forum_thread.php?id='.$thread->id.'"><b>'.cleanup_title($thread->title).'</b></a></td>';
        echo '
                <td>'.($thread->replies+1).'</td>
                <td align="left"><div class="authorcol">'.user_links($owner).'</div></td>
                <td>'.$thread->views.'</td>
                <td style="text-align:right">'.time_diff_str($thread->timestamp, time()).'</td>
        </tr>';
    }
    end_table();
    echo "<br /><br />";
}


// Let's see if we can match anything in a post body as well:
//
$posts = search_post_titles(
    $search_list, $forum, $user, $min_timestamp, $limit, $search_sort,
    $show_hidden_posts
);

if (count($posts)){
    echo "<h2>Posts found matching your search query:</h2>";
    start_forum_table(array(tra("Topic"), tra("Threads"), tra("Author"),"<nobr>".tra("Last post")."</nobr>"));
    if ($logged_in_user){
        $options = get_output_options($logged_in_user);
    } else {
        $options = new output_options();
    }
    foreach ($posts as $post){
        $thread = BoincThread::lookup_id($post->thread);
        if (($show_hidden_posts == false) && ($thread->hidden)) continue;
        if (($show_hidden_posts == false) && ($post->hidden)) continue;
        $options->setHighlightTerms($search_list);
        $contents = output_transform($post->content, $options);
        $thread_forum = BoincForum::lookup_id($thread->forum);
        $owner = BoincUser::lookup_id($post->user);
        echo '
            <tr>
            <th>'.cleanup_title($thread_forum->title).'</th>
            <th class="threadline"><a href="forum_thread.php?id='.$thread->id.'"><b>'.cleanup_title($thread->title).'</b></a></th>
                <th align="left"><div class="authorcol">'.user_links($owner).'</div></th>
                <th style="text-align:right">'.time_diff_str($post->timestamp, time()).'</th>
        </tr>
        <tr>
            <td colspan=4>'.substr($contents,0,200).'...
            </td>
        </tr>
        <tr><td colspan=4 class="postfooter">
                <a href="forum_thread.php?id='.$thread->id.'&nowrap=true#'.$post->id.'"><b>[Read the rest of this post]</b></a>
        </td></tr>
        <tr class="postseperator"><td colspan=4>&nbsp;</td></tr>
        ';
    }
    end_table();
}

if (!count($thread) && !count($posts)){
    echo "<p>Sorry, couldn't find anything matching your search query. You 
    can try to broaden your search by using less words (or less specific words).</p>
    <p>You can also 
    <a href=\"http://www.google.com/search?domains=".URL_BASE."&sitesearch=".URL_BASE."/forum_thread.php&q=".htmlentities($search_keywords)."\">
    try the same search on Google.</a></p>";
}
echo "<p><a href=\"forum_search.php\">Perform another search</a></p>";
page_tail();
exit;

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
