<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2021 University of California
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

require_once('../inc/util.inc');
require_once('../inc/forum.inc');
require_once('../inc/forum_email.inc');

if (DISABLE_FORUMS) error_page("Forums are disabled");

check_get_args(array("post", "submit", "reason", "tnow", "ttok"));

$postId = get_int('post');

$post = BoincPost::lookup_id($postId);
if (!$post) error_page("No such post.");
$thread = BoincThread::lookup_id($post->thread);
if (!$thread) error_page("No such thread.");
$forum = BoincForum::lookup_id($thread->forum);
if (!$forum) error_page("No such forum.");

$user = get_logged_in_user();
BoincForumPrefs::lookup($user);
check_banished($user);
if (VALIDATE_EMAIL_TO_POST) {
    check_validated_email($user);
}

// Make sure the user has the forum's minimum amount of RAC and total credit
// before allowing them to report a post.
// Using the same rules as for rating (at least for now)
//

if ($user->total_credit<$forum->rate_min_total_credit || $user->expavg_credit<$forum->rate_min_expavg_credit) {
    error_page(tra("You need more average or total credit to report a post."));
}

// Action part
//
$success_page=0;
if (get_str("submit",true)){
    check_tokens($user->authenticator);
    $reason = get_str("reason");
    if (send_report_post_email($user, $forum, $thread, $post, $reason)){
        $success_page=1;
    } else {
        echo "send email failed";
        $success_page=-1;
    }
}

$no_forum_rating = parse_bool(get_config(), "no_forum_rating");

// Display part
//
if ($success_page==1) {
    page_head(tra("Report Registered"));
    echo tra("Your report has been recorded. Thanks for your input.")."<p>"
        .tra("A moderator will now look at your report and decide what will happen - this may take a little while, so please be patient");

    echo "<p><a href=\"forum_thread.php?id=$thread->id&postid=$post->id\">".tra("Return to thread")."</a>";
} elseif ($success_page==0){
    page_head(tra("Report a forum post"));
    if (!$no_forum_rating) {
        echo "<p>".tra("Before reporting this post, consider using the +/- rating system instead. If enough users rate a post negatively it will eventually be hidden.<br />You can find the rating system at the bottom of the post.")."</p>
        ";
    }
    echo "<form action=\"forum_report_post.php\" method=\"get\">\n";
    echo form_tokens($user->authenticator);
    echo "<input type=\"hidden\" name=\"post\" value=\"".$post->id."\">";

    start_table('table-striped');
    echo "<tr><th width=20% class=\"bg-primary\">".tra("Author")."</th>
        <th class=\"bg-primary\">".tra("Message")."</th></tr>
    ";
    show_post($post, $thread, $forum, $user);
    row1(tra("Report post"));
    echo "<tr><td>
    ";
    echo tra("Why do you find the post offensive: %1 Please include enough information so that a person that has not yet read the thread will quickly be able to identify the issue. %2", "<p><small>", "</small>");
    echo '</td><td>
        <textarea name="reason" rows="12" class="form-control"></textarea>
        </td></tr>
        <tr><td></td><td>
    ';
    echo "<input class=\"btn btn-primary\" type=\"submit\" name=\"submit\" value=\"".tra("OK")."\">";
    echo "</td></tr>";
    end_table();
    echo "</form>";
} elseif ($success_page==-1) {
    page_head(tra("Report not registered"));
    echo "<p>".tra("Your report could not be recorded. Please wait a while and try again.")."</p>
        <p>".tra("If this is not a temporary error, please report it to the project developers.")."</p>
    ";
    echo "<a href=\"forum_thread.php?id=$thread->id&postid=$post->id\">".tra("Return to thread")."</a>";
}
page_tail();
?>
