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

// Post to a thread (possibly replying to an existing post).
// Both form and action are here.
//
// Note: the filename is confusing:
// we "reply" to a post, not a thread

require_once('../inc/util.inc');
require_once('../inc/forum_email.inc');
require_once('../inc/forum.inc');
require_once('../inc/bbcode_html.inc');
require_once('../inc/akismet.inc');

if (DISABLE_FORUMS) error_page("Forums are disabled");

$logged_in_user = get_logged_in_user(true);
BoincForumPrefs::lookup($logged_in_user);
check_banished($logged_in_user);
if (VALIDATE_EMAIL_TO_POST) {
    check_validated_email($logged_in_user);
}

$thread = BoincThread::lookup_id(get_int('thread'));
if (!$thread) error_page('No such thread');
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
    check_tokens($logged_in_user->authenticator);
    if (!akismet_check($logged_in_user, $content)) {
        $warning = tra("Your post has been flagged as spam by the Akismet anti-spam system. Please modify your text and try again.");
        $preview = tra("Preview");
    } else {
        $add_signature = post_str('add_signature', true);
        $post_id = create_post(
            $content, $parent_post_id, $logged_in_user, $forum,
            $thread, $add_signature
        );
        if ($post_id) {
            if (post_str('subscribe', true)) {
                BoincSubscription::replace($logged_in_user->id, $thread->id);
            }
            header("Location: forum_thread.php?id=$thread->id&postid=$post_id");
        } else {
            error_page("Can't create post.  $forum_error");
        }
    }
}

page_head(tra("Post to thread")." '$thread->title'",'','','', $bbcode_js);

if ($parent_post) {
    echo sprintf(
        '<h4>Replying to <a href=#%d>message %d</a></h4>',
        $parent_post->id,
        $parent_post->id
    );
}

if ($warning) {
    echo "<p class=\"text-danger\">$warning</p>";
}

switch ($forum->parent_type) {
case 0:
    $category = BoincCategory::lookup_id($forum->category);
    echo forum_title($category, $forum, $thread);
    break;
case 1:
    echo team_forum_title($forum, $thread);
    break;
}
echo "<p>";

if ($preview == tra("Preview")) {
    $options = new output_options;
    if (is_admin($logged_in_user)) {
        $options->htmlitems = false;
    }
    panel(tra('Preview'),
        function() use($content, $options) {
            echo output_transform($content, $options);
        }
    );
}

start_table();
show_message_row($thread, $parent_post);
end_table();

if ($parent_post) {
    start_forum_table(array(tra("Author"), tra("Message")));
    show_post(
        $parent_post, $thread, $forum, $logged_in_user, 0, false, false
    );
    end_table();
} else {
    show_posts($thread, $forum, 0, 0, CREATE_TIME_NEW, 0, $logged_in_user);
}

page_tail();

function show_message_row($thread, $parent_post) {
    global $logged_in_user, $bbcode_html;
    global $content, $preview;

    $x1 = tra("Message:").bbcode_info().post_warning();
    $x2 = "";
    $x2 .= "<form action=forum_reply.php?thread=".$thread->id;

    if ($parent_post) {
        $x2 .= "&post=".$parent_post->id;
    }

    $x2 .= " method=\"post\" name=\"post\" onsubmit=\"return checkForm(this)\">\n";
    $x2 .= form_tokens($logged_in_user->authenticator);
    $x2 .= $bbcode_html."<textarea class=\"form-control\" name=\"content\" rows=\"18\">";
    $no_quote = get_int("no_quote", true)==1;
    if ($preview) {
        $x2 .= htmlspecialchars($content);
    } else if (!$no_quote) {
        if ($parent_post) {
            $x2 .= quote_text($parent_post)."\n";
        }
    }
    if (!$logged_in_user->prefs->no_signature_by_default) {
        $enable_signature='checked="true"';
    } else {
        $enable_signature="";
    }
    $x2 .= sprintf('</textarea><p> </p>
        <input class="btn btn-sm" %s type="submit" name="preview" value="%s">
        <input class="btn btn-sm" %s type="submit" value="%s">
        &nbsp;&nbsp;&nbsp;
        <input type="checkbox" name="add_signature" id="add_signature" %s>
        <label for="add_signature">%s</label>
        &nbsp;&nbsp;&nbsp;
        <input type="checkbox" name="subscribe" id="subscribe">
        <label for="subscribe">%s</label>
        </form>',
        button_style('blue'),
        tra("Preview"),
        button_style(),
        tra("Post"),
        $enable_signature,
        tra("Add my signature to this post"),
        tra("Subscribe to this thread")
    );
    row2($x1, $x2, false, FORUM_LH_PCT);
}

function quote_text($post) {
    $user = BoincUser::lookup_id($post->user);
    return sprintf(
        'In reply to %s\'s message of %s:
        [quote]%s[/quote]',
        $user?$user->name:'unknown user',
        date_str($post->timestamp),
        htmlspecialchars($post->content)
    );
}

?>
