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

// Edit a post.
// The creator of the post can do this up to MAXIMUM_EDIT_TIME

require_once('../inc/forum.inc');
require_once('../inc/bbcode_html.inc');

check_get_args(array("id", "tnow", "ttok"));

$logged_in_user = get_logged_in_user();
BoincForumPrefs::lookup($logged_in_user);

$postid = get_int("id");
$post = BoincPost::lookup_id($postid);
if (!$post) error_page("no such post");
$thread = BoincThread::lookup_id($post->thread);
if (!$thread) error_page("no such thread");
$forum = BoincForum::lookup_id($thread->forum);

if (!is_moderator($logged_in_user, $forum)) {
    if (time() > $post->timestamp + MAXIMUM_EDIT_TIME) {
        error_page(tra("You can no longer edit this post.<br/>Posts can only be edited at most %1 minutes after they have been created.", (MAXIMUM_EDIT_TIME/60)));
    }
}

$post_owner = BoincUser::lookup_id($post->user);
if (($logged_in_user->id != $post_owner->id) || (can_reply($thread, $forum, $logged_in_user) == false)) {
    error_page (tra("You are not authorized to edit this post."));
}

$thread_owner = BoincUser::lookup_id($thread->owner);

// If this post belongs to the creator of the thread and is at top-level
// (ie. not a response to another post)
// allow the user to modify the thread title
//
$can_edit_title = ($post->parent_post==0 && $thread_owner->id==$logged_in_user->id && !is_banished($logged_in_user));

$content = post_str("content", true);
$title = post_str("title", true);
$preview = post_str("preview", true);

if (post_str('submit',true) && (!$preview)) {
    if (POST_MAX_LINKS
        && link_count($content) > POST_MAX_LINKS
        && !is_moderator($logged_in_user, $forum)
    ) {
        error_page("Can't update post");
    }
    check_tokens($logged_in_user->authenticator);

    $add_signature = (post_str('add_signature', true) == "1")?1:0;
    $content = substr($content, 0, 64000);
    $content = trim($content);
    if (strlen($content)) {
        $content = BoincDb::escape_string($content);
        $now = time();
        $post->update("signature=$add_signature, content='$content', modified=$now");

        if ($can_edit_title){
            $title = trim($title);
            $title = sanitize_tags($title);
            $title = BoincDb::escape_string($title);
            $thread->update("title='$title'");
        }
        header("Location: forum_thread.php?id=$thread->id&postid=$postid");
    } else {
        delete_post($post, $thread, $forum);
        header("Location: forum_forum.php?id=$forum->id");
    }
}

page_head(tra("Edit post"),'','','', $bbcode_js);

switch ($forum->parent_type) {
case 0:
    $category = BoincCategory::lookup_id($forum->category);
    echo forum_title($category, $forum, $thread);
    break;
case 1:
    echo team_forum_title($forum, $thread);
    break;
}

echo "<p></p>";

if ($preview == tra("Preview")) {
    panel(tra('Preview'),
        function() use($content) {
            echo output_transform($content, null);
        }
    );
}

echo sprintf(
    '<form action="forum_edit.php?id=%d" method="POST" name=post onsubmit="return checkForm(this)">',
    $post->id
);
echo form_tokens($logged_in_user->authenticator);
start_table();
row1(tra("Edit your message"));
if ($can_edit_title) {
    row2(
        tra("Title").bbcode_info(),
        sprintf(
            '<input type="text" size=%d name="title" value="%s">',
            80,
            htmlspecialchars($preview?$title:$thread->title)
        ),
        null, FORUM_LH_PCT
    );
}

row2(
    sprintf('%s %s %s',
        tra("Message"),
        bbcode_info(),
        post_warning()
    ),
    sprintf(
        '%s <textarea name="content" rows="%d" cols="%d">%s</textarea>',
        $bbcode_html,
        12,
        80,
        htmlspecialchars($preview?$content:$post->content)
    ),
    null, FORUM_LH_PCT
);

row2("",
    sprintf(
        '<input id="add_signature" name="add_signature" value="1" type="checkbox" %s>
        <label for="add_signature">%s</label>
        ',
        $post->signature?'checked="true"':'',
        tra("Add my signature to this post")
    ),
    null, FORUM_LH_PCT
);
row2("",
    sprintf(
        '<input class="btn btn-primary" type="submit" name="preview" value="%s">
        &nbsp;
        <input class="btn btn-success" type="submit" name="submit" value="OK">',
        tra("Preview")
    ),
    null, FORUM_LH_PCT
);

end_table();

echo "</form>";

page_tail();

?>
