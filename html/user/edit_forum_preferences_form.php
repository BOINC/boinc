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

// This provides the form from which the user can edit his or her
// forum preferences.  It relies upon edit_forum_preferences_action.php
// to do anything.

require_once("../inc/util.inc");
require_once("../inc/forum.inc");

check_get_args(array());

$user = get_logged_in_user();
BoincForumPrefs::lookup($user);

page_head(tra("Community preferences"));

text_counter_script();

start_table();
echo "<form method=\"post\" action=\"edit_forum_preferences_action.php\" enctype=\"multipart/form-data\">";

// ------------ Notification -----------

row1(tra("Notifications"));
$ch0 = $user->prefs->pm_notification==0?"checked":"";
$ch1 = $user->prefs->pm_notification==1?"checked":"";
$ch2 = $user->prefs->pm_notification==2?"checked":"";
row2(
    tra("How should we notify you of new private messages, friend requests, posts in subscribed threads, and other events?"),
    "<input type=radio name=pm_notification value=0 $ch0> ".tra("On my Account page (no email)")."
    <br><input type=radio name=pm_notification value=1 $ch1> ".tra("Immediately, by email")."
    <br><input type=radio name=pm_notification value=2 $ch2> ".tra("In a single daily email")."
    "
);

if (!DISABLE_FORUMS) {
// ------------ Forum identity -----------

$select_0 = $select_1 = $select_2 = "";
if (strlen($user->prefs->avatar)){
    if (substr($user->prefs->avatar, 0, 23) == 'http://www.gravatar.com' || substr($user->prefs->avatar, 0, 18)=="//www.gravatar.com") { // Gravatar
        $select_1 = "checked=\"true\"";
    } else {
        $select_2 = "checked=\"true\"";
    }
} else {
    $select_0 = "checked=\"true\"";
}
row1(tra("Message-board identity"));
row2(tra("Avatar")."
    <br><p class=\"text-muted\">".tra("An image representing you on the message boards.")."
    <br>".tra("Format: JPG or PNG. Size: at most 4 KB, 100x100 pixels")."</p>",
    "<input type=\"radio\" id=\"avatar_select_0\" name=\"avatar_select\" value=\"0\" ".$select_0.">
        <label for=\"avatar_select_0\">".tra("Don't use an avatar")."</label><br>
    <input type=\"radio\" id=\"avatar_select_1\" name=\"avatar_select\" value=\"1\" ".$select_1.">
        <label for=\"avatar_select_1\">".tra("Use a Globally Recognized Avatar provided by %1", "<a href=\"http://gravatar.com\">Gravatar.com</a>")."</label><br>
    <input type=\"radio\" id=\"avatar_select_2\" name=\"avatar_select\" value=\"2\" ".$select_2.">
        <label for=\"avatar_select_2\">".tra("Use this uploaded avatar:")."</label> <input type=\"file\" name=\"picture\">"
);
if (strlen($user->prefs->avatar)){
    row2(tra("Avatar preview")."<br><p class=\"text-muted\">".tra("This is how your avatar will look")."</p>",
    "<img src=\"".$user->prefs->avatar."\" width=\"100\" height=\"100\">");
}

$signature_by_default = $user->prefs->no_signature_by_default==false?"checked=\"checked\"":"";

$signature=$user->prefs->signature;
$maxlen=250;
$x = '';
if (!NO_COMPUTING) {
    $x = tra(
        "Check out %1 various free services %2 <br> providing dynamic 'signature images' <br> showing your latest credit info, project news, etc.",
        '<a href=https://github.com/BOINC/boinc/wiki/WebResources>',
        '</a>'
    );
}
row2(
    sprintf(
        'Signature for message board posts%s<br><br>%s',
        bbcode_info(),
        $x
    ),
    sprintf(
        '%s <br><input type="checkbox" name="signature_by_default" %s> %s',
        textarea_with_counter("signature", 250, $signature),
        $signature_by_default,
        tra("Attach signature by default")
    )
);
if ($user->prefs->signature!=""){
    row2(tra("Signature preview").
        "<br><p class=\"text-muted\">".tra("This is how your signature will look in the forums")."</p>",
        output_transform($user->prefs->signature)
    );
}

// ------------ Message display  -----------

$forum_hide_avatars = $user->prefs->hide_avatars?"checked=\"checked\"":"";
$forum_hide_signatures = $user->prefs->hide_signatures?"checked=\"checked\"":"";
$forum_link_popup = $user->prefs->link_popup?"checked=\"checked\"":"";
$forum_image_as_link = $user->prefs->images_as_links?"checked=\"checked\"":"";
$forum_jump_to_unread = $user->prefs->jump_to_unread?"checked=\"checked\"":"";
$forum_ignore_sticky_posts = $user->prefs->ignore_sticky_posts?"checked=\"checked\"":"";
$forum_highlight_special = $user->prefs->highlight_special?"checked=\"checked\"":"";

$forum_minimum_wrap_postcount = intval($user->prefs->minimum_wrap_postcount);
$forum_display_wrap_postcount = intval($user->prefs->display_wrap_postcount);

row1(tra("Message display"));
row2(
    tra("What to display"),
    "<input type=\"checkbox\" name=\"forum_hide_avatars\" ".$forum_hide_avatars."> ".tra("Hide avatar images")."<br>
    <input type=\"checkbox\" name=\"forum_hide_signatures\" ".$forum_hide_signatures."> ".tra("Hide signatures")."<br>
    <input type=\"checkbox\" name=\"forum_images_as_links\" ".$forum_image_as_link."> ".tra("Show images as links")."<br>
    <input type=\"checkbox\" name=\"forum_link_popup\" ".$forum_link_popup."> ".tra("Open links in new window/tab")."<br>
    <input type=\"checkbox\" name=\"forum_highlight_special\" ".$forum_highlight_special."> ".tra("Highlight special users")."<br>
    <input type=\"text\" name=\"forum_display_wrap_postcount\" size=3 value=\"".$forum_display_wrap_postcount."\"> ".tra("Display this many messages per page")."<br />
    "
);

row2(tra("How to sort"),
    tra("Threads:")." ".select_from_array("forum_sort", $forum_sort_styles, $user->prefs->forum_sorting)."<br>".tra("Posts:")." ".select_from_array("thread_sort", $thread_sort_styles, $user->prefs->thread_sorting)."<br>
    <input type=\"checkbox\" name=\"forum_jump_to_unread\" ".$forum_jump_to_unread."> ".tra("Jump to first new post in thread automatically")."<br>
    <input type=\"checkbox\" name=\"forum_ignore_sticky_posts\" ".$forum_ignore_sticky_posts."> ".tra("Don't move sticky posts to top")."<br>
    "
);
}   // DISABLE_FORUMS

// ------------ Message filtering  -----------

row1(tra("Message blocking"));

// get list of blocked users
//
$blocked_users = get_ignored_list($user);
$blocked_str = "";
foreach ($blocked_users as $id) {
    if ($id) {
        $blocked_user = BoincUser::lookup_id((int)$id);
        if (!$blocked_user) {
            echo "Missing user $id";
            continue;
        }
        $blocked_str .= sprintf(
            '
                %s %s
                <input class="btn btn-default" type="submit" name="remove%d" value="%s">
                <br>
            ',
            UNIQUE_USER_NAME?'':"$blocked_user->id -",
            user_links($blocked_user),
            $blocked_user->id,
            tra("Unblock")
        );
    }
}

row2(
    sprintf(
        '%s<br><p class="text-muted">%s</p>',
        tra("Blocked users"),
        tra('Ignore message board posts and private messages from these users.')
    ),
    $blocked_str?$blocked_str:'---'
);
row2(
    tra('Block user'),
    sprintf(
        '
            %s
            <input type="text" name="forum_filter_user" size=12>
            <input class="btn btn-default" type="submit" name="add_user_to_filter" value="%s">
        ',
        UNIQUE_USER_NAME?tra('User name'):tra('User ID (For instance: 123456789)'),
        tra("Block")
    )
);

row1(tra("Update"));
row2(
    tra("Click here to update preferences"),
    "<input class=\"btn btn-success\" type=submit value=\"".tra("Update")."\">"
);
echo "</form>\n";
row1(tra("Reset"));
row2(tra("Or click here to reset preferences to the defaults"),
    "<form method=\"post\" action=\"edit_forum_preferences_action.php\"><input class=\"btn btn-warning\" type=\"submit\" value=\"".tra("Reset")."\"><input type=\"hidden\" name=\"action\" value=\"reset_confirm\"></form>"
);
end_table();
page_tail();

?>
