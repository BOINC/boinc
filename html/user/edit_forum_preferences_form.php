<?php

// This provides the form from which the user can edit his or her
// forum preferences.  It relies upon edit_forum_preferences_action.php
// to do anything.

require_once("../inc/forum.inc");

$user = get_logged_in_user();
BoincForumPrefs::lookup($user);

page_head("Edit message board preferences");
echo "<script type=\"text/javascript\">
function textCounter(field, countfield, maxlimit)
{
/*
* Input-Parameter: field name;
* the remaining count field;
* max. Characters.
*/
if (field.value.length > maxlimit) // If the input length is greater than allowed
field.value =field.value.substring(0, maxlimit); // no typing is allowed
else
countfield.value = maxlimit - field.value.length // the number of the remaining chars is displayed
} 
</script>";

start_table();
row1("Reset preferences");
row2("<font size=-2>Use this button to reset preferences to the defaults</font>",
    "<form method=\"post\" action=\"edit_forum_preferences_action.php\"><input type=\"submit\" value=\"Reset preferences\"><input type=\"hidden\" name=\"action\" value=\"reset\"></form>");
echo "<form method=\"post\" action=\"edit_forum_preferences_action.php\" enctype=\"multipart/form-data\">";
if (strlen($user->prefs->avatar)){
    $two_select="checked=\"true\"";
} else {
    $zero_select="checked=\"true\"";
}
row1("Avatar");
row2("<font size=-2>The virtual representation of you on the message boards<br/>Note: Forced size of 100x100 pixels<br>format: jpg/png - size: at most 4k</font>",
    "
        <table>
            <tr><td><input type=\"radio\" name=\"avatar_select\" value=\"0\" ".$zero_select.">Don't use an avatar</td><td></td></tr>
            <tr><td><input type=\"radio\" name=\"avatar_select\" value=\"2\" ".$two_select.">Use this uploaded avatar: <input type=\"file\" name=\"picture\"></td><td></td></tr>
        </table>
        "
);
if (strlen($user->prefs->avatar)){
    row2("Avatar preview<br><font size=-2>This is how your avatar will look</font>",
    "<img src=\"".$user->prefs->avatar."\" width=\"100\" height=\"100\">");
}

row1("Sort styles");
row2("<font size=-2>How to sort the replies in the message board and Q&amp;A areas</font>",
    "
        <table>
            <tr><td>Message threadlist:</td><td>".select_from_array("forum_sort", $forum_sort_styles, $user->prefs->forum_sorting)."</td></tr>
            <tr><td>Message posts:</td><td>".select_from_array("thread_sort", $thread_sort_styles, $user->prefs->thread_sorting)."</td></tr>
        </table>"
);

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

$forum_minimum_wrap_postcount = intval($user->minimum_wrap_postcount);
$forum_display_wrap_postcount = intval($user->display_wrap_postcount);

row1("Display and Behavior");
row2(
    "<br><font size=-2>How to treat links and images in the forum<br>and how to act on unread posts</font>",
    "<table><tr><td>
        <input type=\"checkbox\" name=\"forum_images_as_links\" ".$forum_image_as_link."> Show images as links<br>
        <input type=\"checkbox\" name=\"forum_link_popup\" ".$forum_link_popup."> Open links in new window/tab<br>
        <input type=\"checkbox\" name=\"forum_jump_to_unread\" ".$forum_jump_to_unread."> Jump to first new post in thread automatically<br>
        <input type=\"checkbox\" name=\"forum_ignore_sticky_posts\" ".$forum_ignore_sticky_posts.">Do not reorder sticky posts<br>
        <br />
        <input type=\"text\" name=\"forum_minimum_wrap_postcount\" style=\"width: 30px;\" value=\"".$forum_minimum_wrap_postcount."\"> If a thread contains more than this number of posts<br />
        <input type=\"text\" name=\"forum_display_wrap_postcount\" style=\"width: 30px;\" value=\"".$forum_display_wrap_postcount."\"> only display the first one and this many of the last ones<br />
        
    </td></tr></table>"
);
if ($user->prefs->pm_notification){
    $pm_notification="checked=\"checked\"";
} else {
    $pm_notification="";
}
row2("Private message email notification",
    "<table><tr><td><input type=\"checkbox\" id=\"pm_notification\" name=\"pm_notification\" ".$pm_notification.">
        <label for=\"pm_notification\">Notify about new private messages by email</label>
    </td></tr></table>");

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
$forum_low_rating_threshold = $user->prefs->low_rating_threshold;
$forum_high_rating_threshold = $user->prefs->high_rating_threshold;

row1("Filtering");
row2(
    "<br><font size=-2>What to display<br>If you set both your high and low thresholds to 0 or<br>empty they will reset to the default values</font>",
    "<table><tr><td>
        <input type=\"checkbox\" name=\"forum_hide_avatars\" ".$forum_hide_avatars."> Hide avatar images<br>
        <input type=\"checkbox\" name=\"forum_hide_signatures\" ".$forum_hide_signatures."> Hide signatures<br>
    </td></tr></table>
    <table width=\"380\">
        <tr><td width=\"32\"><input type=\"text\" name=\"forum_low_rating_threshold\" value=\"".$forum_low_rating_threshold."\" style=\"width: 30px;\"></td><td>Filter threshold (default: ".DEFAULT_LOW_RATING_THRESHOLD.")</td></tr>
        <tr><td><input type=\"text\" name=\"forum_high_rating_threshold\" value=\"".$forum_high_rating_threshold."\" style=\"width: 30px;\"></td><td>Emphasize threshold (default: ".DEFAULT_HIGH_RATING_THRESHOLD.")</td></tr>
        <tr><td colspan=2>
            Anything rated lower than the filter threshold will be filtered and anything rated higher than the emphasize threshold will be emphasized.
        </td></tr>
    </table>
    "
);

$filtered_userlist = get_ignored_list($user);
for ($i=0; $i<sizeof($filtered_userlist); $i++){
    if ($filtered_userlist[$i] != ""){
        $filtered_user = BoincUser::lookup_id($filtered_userlist[$i]);
        if ($filtered_user) {
            echo "missing user $filtered_userlist[$i]";
            continue;
        }
        $forum_filtered_userlist .= "<input type =\"submit\" name=\"remove".$filtered_user->id."\" value=\"Remove\"> ".$filtered_user->id." - ".user_links($filtered_user)."<br>";
    }
}
row2("Filtered users".
    "<br><font size=-2>Ignore specific users<br>You can define a list of users to ignore.<br>These users will have to write posts with very high<br> rating in order to not be filtered.</font>",
    "<table><tr><td>
        $forum_filtered_userlist
    </td></tr></table>
    <table width=\"380\">
        <tr><td width=\"32\"><input type=\"text\" name=\"forum_filter_user\" style=\"width: 80px;\"></td><td>Userid (For instance: 123456789)</td></tr>
        <tr><td colspan=\"2\"><input type=\"submit\" name=\"add_user_to_filter\" value=\"Add user to filter\"></td></tr>
        <tr><td colspan=2>
            Please note that you can only filter a limited number of users.
        </td></tr>
    </table>
    "
);

if (!$user->prefs->no_signature_by_default){
    $signature_by_default="checked=\"checked\"";
} else {
    $signature_by_default="";
}
$signature=stripslashes($user->prefs->signature);
$maxlen=250;
row1("Signature");
row2(html_info().
    "<font size=-2><br>Max length (including newlines) is $maxlen chars.</font>",
    "<table><tr><td>
    <textarea name=\"signature\" rows=4 cols=50 id=\"signature\" onkeydown=\"textCounter(this.form.signature, this.form.remLen,$maxlen);\"
    onkeyup=\"textCounter(this.form.signature, this.form.remLen,250);\">".$signature."</textarea>
    <br><input name=\"remLen\" type=\"text\" id=\"remLen\" value=\"".($maxlen-strlen($signature))."\" size=\"3\" maxlength=\"3\" readonly> chars remaining
    <br><input type=\"checkbox\" name=\"signature_by_default\" ".$signature_by_default."> Attach signature by default
    </td></tr></table>");
if ($user->prefs->signature!=""){
row2("Signature preview".
    "<br><font size=-2>This is how your signature will look in the forums</font>",
    output_transform($user->prefs->signature)
);
}
row1("&nbsp;");
row2("", "<input type=submit value='Update info'>");
echo "</form>\n";
end_table();
page_tail();

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
