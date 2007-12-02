<?php

// This provides the form from which the user can edit his or her
// forum preferences.  It relies upon edit_forum_preferences_action.php
// to do anything.

require_once("../inc/forum.inc");

$user = get_logged_in_user();
BoincForumPrefs::lookup($user);

page_head("Community preferences");

// output a script for counting chars left in text field
//
echo "<script type=\"text/javascript\">
    function textCounter(field, countfield, maxlimit) {
        if (field.value.length > maxlimit) {
            field.value =field.value.substring(0, maxlimit);
        } else {
            countfield.value = maxlimit - field.value.length
        }
    }
    </script>
";

start_table();
echo "<form method=\"post\" action=\"edit_forum_preferences_action.php\" enctype=\"multipart/form-data\">";

$zero_select = $two_select = "";
if (strlen($user->prefs->avatar)){
    $two_select="checked=\"true\"";
} else {
    $zero_select="checked=\"true\"";
}
row1("Identity");
row2("Avatar
    <br><span class=note>An image representing you.
    <br>Format: JPG or /PNG.  Size: at most 4 KB, 100x100 pixels</span>",
    "<input type=\"radio\" name=\"avatar_select\" value=\"0\" ".$zero_select.">Don't use an avatar <br><input type=\"radio\" name=\"avatar_select\" value=\"2\" ".$two_select.">Use this uploaded avatar: <input type=\"file\" name=\"picture\">"
);
if (strlen($user->prefs->avatar)){
    row2("Avatar preview<br><span class=note>This is how your avatar will look</span>",
    "<img src=\"".$user->prefs->avatar."\" width=\"100\" height=\"100\">");
}

if (!$user->prefs->no_signature_by_default){
    $signature_by_default="checked=\"checked\"";
} else {
    $signature_by_default="";
}
$signature=stripslashes($user->prefs->signature);
$maxlen=250;
row2(
    "Signature<br>
    <span class=note>Max length is $maxlen chars.</span>".
    html_info(),
    "<textarea name=\"signature\" rows=4 cols=50 id=\"signature\" onkeydown=\"textCounter(this.form.signature, this.form.remLen,$maxlen);\"
    onkeyup=\"textCounter(this.form.signature, this.form.remLen,250);\">".$signature."</textarea>
    <br><input name=\"remLen\" type=\"text\" id=\"remLen\" value=\"".($maxlen-strlen($signature))."\" size=\"3\" maxlength=\"3\" readonly> chars remaining
    <br><input type=\"checkbox\" name=\"signature_by_default\" ".$signature_by_default."> Attach signature by default "
);
if ($user->prefs->signature!=""){
    row2("Signature preview".
        "<br><span class=note>This is how your signature will look in the forums</span>",
        output_transform($user->prefs->signature)
    );
}

row1("Private message notification");
if ($user->prefs->pm_notification){
    $pm_notification="checked=\"checked\"";
} else {
    $pm_notification="";
}
row2(
    "Send email notification of new private messages",
    "<input type=\"checkbox\" id=\"pm_notification\" name=\"pm_notification\" ".$pm_notification.">"
);

if ($user->prefs->hide_avatars){
    $forum_hide_avatars = "checked=\"checked\"";
} else {
    $forum_hide_avatars = "";
}
if ($user->prefs->hide_signatures){
    $forum_hide_signatures = "checked=\"checked\"";
} else {
    $forum_hide_signatures = "";
}

if ($user->prefs->link_popup){
    $forum_link_popup="checked=\"checked\"";
} else {
    $forum_link_popup="";
}
if ($user->prefs->images_as_links){
    $forum_image_as_link="checked=\"checked\"";
} else {
    $forum_image_as_link="";
}
if ($user->prefs->jump_to_unread){
    $forum_jump_to_unread="checked=\"checked\"";
} else {
    $forum_jump_to_unread="";
}
if ($user->prefs->ignore_sticky_posts){
    $forum_ignore_sticky_posts="checked=\"checked\"";
} else {
    $forum_ignore_sticky_posts="";
}

$forum_minimum_wrap_postcount = intval($user->prefs->minimum_wrap_postcount);
$forum_display_wrap_postcount = intval($user->prefs->display_wrap_postcount);

row1("Message display");
row2(
    "What to display",
    "<input type=\"checkbox\" name=\"forum_hide_avatars\" ".$forum_hide_avatars."> Hide avatar images<br>
    <input type=\"checkbox\" name=\"forum_hide_signatures\" ".$forum_hide_signatures."> Hide signatures<br>
    <input type=\"checkbox\" name=\"forum_images_as_links\" ".$forum_image_as_link."> Show images as links<br>
    <input type=\"checkbox\" name=\"forum_link_popup\" ".$forum_link_popup."> Open links in new window/tab<br>
    "
);

row2("How to sort",
    "Threads: ".select_from_array("forum_sort", $forum_sort_styles, $user->prefs->forum_sorting)."<br>Posts: ".select_from_array("thread_sort", $thread_sort_styles, $user->prefs->thread_sorting)."<br>
    <input type=\"checkbox\" name=\"forum_jump_to_unread\" ".$forum_jump_to_unread."> Jump to first new post in thread automatically<br>
    <input type=\"checkbox\" name=\"forum_ignore_sticky_posts\" ".$forum_ignore_sticky_posts.">Do not reorder sticky posts<br>
    <input type=\"text\" name=\"forum_minimum_wrap_postcount\" size=3 value=\"".$forum_minimum_wrap_postcount."\"> If a thread contains more than this number of posts<br />
    <input type=\"text\" name=\"forum_display_wrap_postcount\" size=3 value=\"".$forum_display_wrap_postcount."\"> only display the first one and this many of the last ones<br />
    "
);

row1("Message filtering");

$filtered_userlist = get_ignored_list($user);
$forum_filtered_userlist = "";
for ($i=0; $i<sizeof($filtered_userlist); $i++){
    $id = (int)$filtered_userlist[$i];
    if ($id) {
        $filtered_user = BoincUser::lookup_id($id);
        if (!$filtered_user) {
            echo "Missing user $id";
            continue;
        }
        $forum_filtered_userlist .= "<input type =\"submit\" name=\"remove".$filtered_user->id."\" value=\"Remove\"> ".$filtered_user->id." - ".user_links($filtered_user)."<br>";
    }
}
row2("Filtered users".
    "<br><span class=note>Ignore message board posts and private messages from these  users.</span>",
    "$forum_filtered_userlist
    <br>
        <input type=\"text\" name=\"forum_filter_user\" size=12> User ID (For instance: 123456789)
        <br><input type=\"submit\" name=\"add_user_to_filter\" value=\"Add user to filter\">
    "
);

row1("Update");
row2("Click here to update message board preferences", "<input type=submit value=\"Update\">");
echo "</form>\n";
row1("Reset");
row2("Or click here to reset preferences to the defaults",
    "<form method=\"post\" action=\"edit_forum_preferences_action.php\"><input type=\"submit\" value=\"Reset\"><input type=\"hidden\" name=\"action\" value=\"reset_confirm\"></form>"
);
end_table();
page_tail();

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
