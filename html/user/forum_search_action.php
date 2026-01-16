<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// search for posts or a thread.
// Takes input from forum_search.php

require_once('../inc/time.inc');
require_once('../inc/text_transform.inc');
require_once('../inc/forum.inc');

if (DISABLE_FORUMS) error_page("Forums are disabled");

check_get_args(array());

// Searches for the keywords in the $keyword_list array in thread titles.
// Optionally filters by forum, user, time, or hidden if specified.
//
function search_thread_titles(
    $keyword_list, $forum="", $user="", $time="", $limit=200,
    $sort_style=CREATE_TIME_NEW, $show_hidden = false
){
    $search_string="%";
    foreach ($keyword_list as $key => $word) {
        $search_string .= BoincDb::escape_string($word)."%";
    }
    $query = "title like '".$search_string."'";
    if ($forum && $forum != "all") {
        $query .= " and forum = $forum->id";
    }
    if ($user && $user != "all") {
        $query .= " and owner = $user->id";
    }
    if ($time && $user != "all") {
        $query .= " and timestamp > $time";
    }
    if (!$show_hidden) {
        $query .= " and thread.hidden = 0";
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

    $query .= " limit $limit";
    return BoincThread::enum($query);
}

// Searches for the keywords in the $keyword_list array in post bodies.
// optionally filters by forum, time, hidden, or user if specified.
//
function search_post_content(
    $keyword_list, $forum, $user, $time, $limit, $sort_style, $show_hidden
){
    $db = BoincDb::get();

    $search_string="%";
    foreach ($keyword_list as $key => $word){
        $search_string .= BoincDb::escape_string($word)."%";
    }
    $optional_join = "";
    // if looking in a single forum, need to join w/ thread table
    // because that's where the link to forum is
    //
    if ($forum) {
        $optional_join = " LEFT JOIN ".$db->db_name.".thread ON post.thread = thread.id";
    }
    $query = "select post.* from ".$db->db_name.".post".$optional_join." where content like '".$search_string."'";
    if ($forum) {
        $query.=" and forum = $forum->id";
    }
    if ($user) {
        $query.=" and post.user = $user->id ";
    }
    if ($time) {
        $query.=" and post.timestamp > $time";
    }
    if (!$show_hidden) {
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
    $query.= " limit $limit";
    return BoincPost::enum_general($query);
}

$logged_in_user = get_logged_in_user(false);
BoincForumPrefs::lookup($logged_in_user);
if ($logged_in_user && $logged_in_user->prefs->privilege(S_MODERATOR)){
    $show_hidden_posts = true;
} else {
    $show_hidden_posts = false;
}

page_head(tra("Forum search results"));

$search_keywords = post_str("search_keywords", true);
$search_author = post_int("search_author", true);
$search_max_time = post_int("search_max_time");
$search_forum = post_int("search_forum");
$search_sort = post_int("search_sort");
$search_list = explode(" ",$search_keywords);
if ($search_max_time) {
    $min_timestamp = time() - ($search_max_time*3600*24);
} else {
    $min_timestamp = 0;
}

$limit = 100;

if ($search_forum==-1){
    $forum = null;
} else if ($search_forum) {
    $forum = BoincForum::lookup_id($search_forum);
}

$user = null;
if ($search_author) {
    $user = BoincUser::lookup_id($search_author);
}

// First search thread titles; if we get a hit there it's probably relevant
//
$threads = search_thread_titles(
    $search_list, $forum, $user, $min_timestamp, $limit,
    $search_sort, $show_hidden_posts
);

// Display the threads
//
if (count($threads)){
    echo "<h3>" . tra("Thread titles matching your query:") . "</h3>";
    start_table('table-striped');
    thread_list_header();
    foreach ($threads as $thread){
        if (!$show_hidden_posts) {
            $u = BoincUser::lookup_id($thread->owner);
            if ($u && is_banished($u)) continue;
            if ($thread->hidden) continue;
        }
        thread_list_item($thread, $logged_in_user);
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
    echo "<h3>" . tra("Messages matching your query:") . "</h3>";
    start_table('table-striped');
    row_heading_array(['Info', 'Post'], ['', 'width=70%']);
    $n = 1;
    $options = get_output_options($logged_in_user);
    $options->setHighlightTerms($search_list);
    foreach ($posts as $post) {
        if (!$show_hidden_posts) {
            $u = BoincUser::lookup_id($post->user);
            if (is_banished($u)) continue;
        }
        $thread = BoincThread::lookup_id($post->thread);
        if (!$thread) continue;
        $forum = BoincForum::lookup_id($thread->forum);
        if (!$forum) continue;
        if (!is_forum_visible_to_user($forum, $logged_in_user)) continue;

        if (!$show_hidden_posts) {
            if ($thread->hidden) continue;
            if ($post->hidden) continue;
        }
        show_post_and_context($post, $thread, $forum, $options, $n);
        $n++;
    }
    end_table();
}

if (!count($threads) && !count($posts)){
    echo "<p>".tra("Sorry, couldn't find anything matching your search query. You can try to broaden your search by using less words (or less specific words).")."</p>
    <p>"
    .tra("You can also %1 try the same search on Google. %2",
         "<a href=\"https://www.google.com/search?domains=".url_base()."&sitesearch=".url_base()."forum_thread.php&q=".htmlentities($search_keywords)."\">",
         "</a>")
    ."</p>";
}
echo "<p><a href=\"forum_search.php\">".tra("Perform another search")."</a></p>";
page_tail();

?>
