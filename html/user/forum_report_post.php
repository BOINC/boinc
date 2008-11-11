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

// Some posts may contain material that is not suited for public
// viewing. This file allows people to report such posts
// For this file to work the project must have defined who
// should receive such reports (in the configuration file)

require_once('../inc/forum.inc');
require_once('../inc/forum_email.inc');


$postId = get_int('post');

$post = BoincPost::lookup_id($postId);
$thread = BoincThread::lookup_id($post->thread);
$forum = BoincForum::lookup_id($thread->forum);

$user = get_logged_in_user();
BoincForumPrefs::lookup($user);
check_banished($user);

// Make sure the user has the forum's minimum amount of RAC and total credit
// before allowing them to report a post.
// Using the same rules as for rating (at least for now)
//

if ($user->total_credit<$forum->rate_min_total_credit || $user->expavg_credit<$forum->rate_min_expavg_credit) {
    error_page("You need more average or total credit to report a post.");
}

//__-------------- Action part
$success_page=0;
if (get_str("submit",true)){
    check_tokens($user->authenticator);
    $reason = get_str("reason");
    if (send_report_post_email($user, $forum, $thread, $post, $reason)){
        $success_page=1;
    } else {
        $success_page=-1;
    }
}

$no_forum_rating = parse_bool($config, "no_forum_rating"); 

//__--------------- Display part
if ($success_page==1) {
    page_head('Report Registered');
    echo "<p>Your report has been successfully recorded.
        Thank you for your input.</p>
        <p>A moderator will now look at your report and decide what will happen -
        this may take a little while, so please be patient</p>
    ";
    echo "<a href=\"forum_thread.php?id=", $thread->id, "#", $post->id, "\">Return to thread</a>";
} elseif ($success_page==0){
    page_head('Report a forum post'); 
    if (!$no_forum_rating) {
        echo "<p>Before reporting this post <em>please</em>
            consider using the +/- rating system instead.
            If enough users agree on rating a post negatively it will
            eventually be hidden.
            <br>You can find the rating system at the bottom of the post.</br>
        ";
    }
    start_forum_table(array(tra("Author"), tra("Message"),""));
    show_post($post, $thread, $forum, $user, 0, 0);
    echo "<form action=\"forum_report_post.php\" method=\"get\">\n";
    echo form_tokens($user->authenticator);
    row1("Report post");
    row2("Why do you find the post offensive:<br><font size=-1>Please include enough information so that a person that 
    has not yet read the thread will quickly be able to identify the issue.</font>",
        "<textarea name=\"reason\" rows=12 cols=54></textarea>"
    );
    row2("", "<input type=\"submit\" name=\"submit\" value=\"OK\">");
    echo "<input type=\"hidden\" name=\"post\" value=\"".$post->id."\">";
    echo "</form>";		    
    end_table();
} elseif ($success_page==-1) {
    page_head('Report NOT registered');
    echo "<p>Your report could not be recorded.
        Please wait a short while and try again.</p>
        <p>If this is not a temporary error,
        please report it to the project developers.</p>
    ";
    echo "<a href=\"forum_thread.php?id=", $thread->id, "#", $post->id, "\">Return to thread</a>";
}
page_tail();
?>
