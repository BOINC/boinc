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

// Post a reply to a thread.
// Both input (form) and action take place here.

require_once('../inc/forum_email.inc');
require_once('../inc/forum.inc');
require_once('../inc/bbcode_html.inc');
require_once('../inc/akismet.inc');

$logged_in_user = get_logged_in_user(true);
BoincForumPrefs::lookup($logged_in_user);
check_banished($logged_in_user);

$thread = BoincThread::lookup_id(get_int('thread'));
$forum = BoincForum::lookup_id($thread->forum);

$sort_style = get_str('sort', true);
$filter = get_str('filter', true);
$content = post_str('content', true);
$preview = post_str("preview", true);
$parent_post_id = get_int('post', true);

$parent_post = null;
if ($parent_post_id) {
    $parent_post = BoincPost::lookup_id($parent_post_id);
    if ($parent_post->thread != $thread->id) {
        error_page("wrong thread");
    }
} else {
    $parent_post_id = 0;
}

if ($filter != "false"){
    $filter = true;
} else {
    $filter = false;
}

check_reply_access($logged_in_user, $forum, $thread);

if (!$sort_style) {
    $sort_style = $logged_in_user->prefs->thread_sorting;
} else {
    $logged_in_user->prefs->update("thread_sorting=$sort_style");
}

$warning = null;
if ($content && (!$preview)){
    if (post_str('add_signature',true)=="add_it"){
        $add_signature=true;    // set a flag and concatenate later
    }  else {
        $add_signature=false;
    }
    check_tokens($logged_in_user->authenticator);
    if (!akismet_check($logged_in_user, $content)) {
        $warning = tra("Your post has been flagged as spam by the Akismet anti-spam system. Please modify your text and try again.");
        $preview = tra("Preview");
    } else {
        create_post(
            $content, $parent_post_id, $logged_in_user, $forum,
            $thread, $add_signature
        );
        header('Location: forum_thread.php?id='.$thread->id);
    }
}

page_head(tra("Post to thread"),'','','', $bbcode_js);

show_forum_header($logged_in_user);

if ($warning) {
    echo "<span class=error>$warning</span><p>";
}

switch ($forum->parent_type) {
case 0:
    $category = BoincCategory::lookup_id($forum->category);
    show_forum_title($category, $forum, $thread);
    break;
case 1:
    show_team_forum_title($forum, $thread);
    break;
}
echo "<p>";

if ($preview == tra("Preview")) {
    $options = new output_options;
    echo "<h2>".tra("Preview")."</h2>\n";
    echo "<div class=\"pm_preview\">"
        .output_transform($content, $options)
        ."</div>\n"
    ;
}

start_forum_table(array(tra("Author"), tra("Message")));

show_message_row($thread, $parent_post);
if ($parent_post) {
    show_post(
        $parent_post, $thread, $forum, $logged_in_user, 0, 0, false, false
    );
}
end_table();

page_tail();

function show_message_row($thread, $parent_post) {
    global $logged_in_user, $bbcode_html;
    global $content, $preview;

    $x1 = tra("Message:").html_info().post_warning();
    $x2 = "";
    if ($parent_post) {
        $x2 .=" ".tra("reply to %1Message ID%2:", "<a href=#".$parent_post->id.">", " ".$parent_post->id."</a>");
    }
    $x2 .= "<form action=forum_reply.php?thread=".$thread->id;

    if ($parent_post) {
        $x2 .= "&post=".$parent_post->id;
    }

    $x2 .= " method=\"post\" name=\"post\" onsubmit=\"return checkForm(this)\">\n";
    $x2 .= form_tokens($logged_in_user->authenticator);
    $x2 .= $bbcode_html."<textarea name=\"content\" rows=\"18\" cols=\"80\">";
    $no_quote = get_int("no_quote", true)==1;
    if ($preview) {
        $x2 .= htmlspecialchars($content);
    } else if (!$no_quote) {
        if ($parent_post) $x2 .= quote_text(htmlspecialchars($parent_post->content))."\n";
    }
    if (!$logged_in_user->prefs->no_signature_by_default) {
        $enable_signature="checked=\"true\"";
    } else {
        $enable_signature="";
    }
    $x2 .= "</textarea><p>
        <input type=\"submit\" name=\"preview\" value=\"".tra("Preview")."\">
        <input type=\"submit\" value=\"".tra("Post reply")."\">
        &nbsp;&nbsp;&nbsp;
        <input type=\"checkbox\" name=\"add_signature\" id=\"add_signature\" value=\"add_it\" ".$enable_signature.">
        <label for=\"add_signature\">".tra("Add my signature to this reply")."</label>

        </form>
    ";
    row2($x1, $x2);
}

function quote_text($text) {
    $text = "[quote]" . $text . "[/quote]";
    return $text;
}

$cvs_version_tracker[]="\$Id$";
?>
