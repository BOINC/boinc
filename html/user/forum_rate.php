<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
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

// This file allows people to rate posts in a thread

require_once('../inc/forum.inc');
require_once('../inc/util.inc');

if (DISABLE_FORUMS) error_page("Forums are disabled");

$config = get_config();
if (parse_bool($config, "no_forum_rating")) {
    error_page("disabled");
}

if (!empty($_GET['post'])) {
    $post_id = get_int('post');
    $choice = post_str('submit', true);
    $rating = post_int('rating', true);
    if (!$choice) $choice = get_str('choice', true);

    if ($choice == SOLUTION or $choice=="p") {
        $rating = 1;
    } else {
        $rating = -1;
    }

    $user = get_logged_in_user();

    if ($choice == null && ($rating == null || $rating > 2 || $rating < -2)) {
        show_result_page(false, NULL, NULL, $choice);
    }

    $post = BoincPost::lookup_id($post_id);
    if (!$post) error_page('No such post');
    $thread = BoincThread::lookup_id($post->thread);
    $forum = BoincForum::lookup_id($thread->forum);

    // Make sure the user has the forum's minimum amount of RAC and total credit
    // before allowing them to rate a post.
    //
    if ($user->total_credit<$forum->rate_min_total_credit || $user->expavg_credit<$forum->rate_min_expavg_credit) {
        error_page(tra("You need more average or total credit to rate a post."));
    }

    if (BoincPostRating::lookup($user->id, $post->id)) {
        error_page(tra("You have already rated this post.")."<br /><br /><a href=\"forum_thread.php?nowrap=true&id=$thread->id&postid=$post->id\">".tra("Return to thread")."</a>");
    } else {
        $success = BoincPostRating::replace($user->id, $post->id, $rating);
        show_result_page($success, $post, $thread, $choice);
    }
}

function show_result_page($success, $post, $thread, $choice) {
    if ($success) {
        if ($choice) {
            page_head(tra("Input Recorded"));
            echo tra("Your input has been recorded. Thanks for your help.");
        } else {
            page_head(tra("Vote Registered"));
            echo tra("Your rating has been recorded. Thanks for your input.");
        }
        echo "<p><a href=\"forum_thread.php?nowrap=true&id=$thread->id&postid=$post->id\">".tra("Return to thread")."</a>";
    } else {
        page_head(tra("Vote Submission Problem"));
        if ($post) {
            echo "There was a problem recording your vote in our database. Please try again later.";
            echo "<a href=\"forum_thread.php?id=$thread->id&postid=$post->id\">".tra("Return to thread")."</a>";
        } else {
            echo "The post you specified does not exist, or your rating was invalid.";
        }
    }
    page_tail();
    exit;
}

?>
