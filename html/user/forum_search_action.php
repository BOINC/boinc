<?php
// search for posts or a thread.
// Takes input from forum_search.php
 
require_once('../inc/time.inc');
require_once('../inc/text_transform.inc');
require_once('../inc/forum.inc');

// Searches for the keywords in the $keyword_list array in thread titles.
// Optionally filters by forum, user, time, or hidden if specified.
//
function search_thread_titles(
    $keyword_list, $forum="", $user="", $time="", $limit=200,
    $sort_style=CREATE_TIME_NEW, $show_hidden = false
){
    $search_string="%";
    foreach ($keyword_list as $key => $word) {
        $search_string.=mysql_escape_string($word)."%";
    }        
    $query = "title like '".$search_string."'";
    if ($forum!="" && $forum!="all") {
        $query.=" and forum = ".intval($forum->id);
    }
    if ($user!="" && $user!="all") {
        $query.=" and owner = ".intval($user->id);
    }
    if ($time!="" && $user!="all") {
        $query.=" and timestamp > ".intval($time);
    }
    if ($show_hidden == false) {
        $query .= " AND thread.hidden = 0";
    }
    switch($sort_style) {
    case MODIFIED_NEW:
        $query .= ' ORDER BY timestamp DESC';
        break;
    case VIEWS_MOST:
        $query .= ' ORDER BY views DESC';
        break;
    case REPLIES_MOST:
        $query .= ' ORDER BY replies DESC';
        break;
    case CREATE_TIME_NEW:
        $query .= ' ORDER by create_time desc';
        break;
    case CREATE_TIME_OLD:
        $query .= ' ORDER by create_time asc';
        break;
    case 'score':
        $query .= ' ORDER by score desc';
        break;
    default:
        $query .= ' ORDER BY timestamp DESC';
        break;
    }

    $query.= " limit ".intval($limit);
    return BoincThread::enum($query);
}

// Searches for the keywords in the $keyword_list array in post bodies.
// optionally filters by forum, time, hidden, or user if specified.
//
function search_post_content(
    $keyword_list, $forum="", $user="", $time="", $limit=200,
    $sort_style=CREATE_TIME_NEW, $show_hidden = false
){
    $search_string="%";
    foreach ($keyword_list as $key => $word){
        $search_string.=mysql_escape_string($word)."%";
    }
    $optional_join = "";
    if ($forum!="" && $forum!="all"){
        $optional_join = " LEFT JOIN DBNAME.thread ON post.thread = thread.id";
    }
    $query = "select *,post.id as postid from DBNAME.post".$optional_join." where content like '".$search_string."'";
    if ($forum!="" && $forum!="all"){
        $query.=" and forum = ".intval($forum->id);
    }
    if ($user!="" && $user!="all"){
        $query.=" and post.user = ".intval($user->id );
    }
    if ($time!="" && $user!="all"){
        $query.=" and post.timestamp > ".intval($time);
    }
    if ($show_hidden == false) {
        $query .= " AND post.hidden = 0";
    }
    switch($sort_style) {
    case VIEWS_MOST:
        $query.= ' ORDER BY views DESC';
        break;
    case CREATE_TIME_NEW:
        $query .= ' ORDER by post.timestamp desc';
        break;
    case CREATE_TIME_OLD:
        $query .= ' ORDER by post.timestamp asc';
        break;
    case POST_SCORE:
        $query .= ' ORDER by post.score desc';
        break;
    default:
        $query .= ' ORDER BY post.timestamp DESC';
        break;
    }
    $query.= " limit ".intval($limit);
    return BoincPost::enum_general($query);
}

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

// First search thread titles; if we get a hit there it's probably relevant
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


// Look in a post content as well
//
$posts = search_post_content(
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
