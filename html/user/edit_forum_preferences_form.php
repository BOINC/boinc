<?php

require_once("../inc/db.inc");
require_once("../inc/forum.inc");
require_once("../inc/util.inc");

db_init();
$user = get_logged_in_user();
$user = getForumPreferences($user);

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
row2("Reset preferences<br><font size=-2>Use this button to reset preferences to the defaults</font>",
    "<form method=\"post\" action=\"edit_forum_preferences_action.php\"><input type=\"submit\" value=\"Reset preferences\"></form>");
echo "<form method=\"post\" action=\"edit_forum_preferences_action.php\" enctype=\"multipart/form-data\">";
if ($user->avatar_type==0){
    $zero_select="checked=\"checked\"";
} elseif($user->avatar_type==1){
    $one_select="checked=\"checked\"";
    $avatar_url=$user->avatar;
} elseif($user->avatar_type==2){
    $two_select="checked=true";
}

row2("Avatar<br><font size=-2>The virtual representation of you on the message boards<br/>Note: Forced size of 100x100 pixels<br>format: jpg/png - size: at most 4k</font>",
    "
        <table>
            <tr><td><input type=\"radio\" name=\"avatar_select\" value=\"0\" ".$zero_select.">Don't use an avatar</td><td></td></tr>
            <!--<tr><td><input type=\"radio\" name=\"avatar_select\" value=\"1\" ".$one_select.">Use external avatar:</td><td><input name=\"avatar_url\" size=30 value=\"".$avatar_url."\"'></td></tr>-->
            <tr><td><input type=\"radio\" name=\"avatar_select\" value=\"2\" ".$two_select.">Use this uploaded avatar: <input type=\"file\" name=\"picture\"></td><td></td></tr>
        </table>
        "
);
if ($user->avatar!=""){
    row2("Avatar preview<br><font size=-2>This is how your avatar will look</font>",
    "<img src=\"".$user->avatar."\" width=\"100\" height=\"100\">");
}

row2("Sort styles<br><font size=-2>How to sort the replies in the message board and Q&amp;A areas</font>",
    "
        <table>
            <tr><td>Message threadlist:</td><td>".select_from_array("forum_sort", $forum_sort_styles, getSortStyle($user,"forum"))."</td></tr>
            <tr><td>Message posts:</td><td>".select_from_array("thread_sort", $thread_sort_styles, getSortStyle($user,"thread"))."</td></tr>
            <tr><td>Q&amp;A questionlist:</td><td>".select_from_array("faq_sort", $faq_sort_styles,  getSortStyle($user,"faq"))."</td></tr>
            <tr><td>Q&amp;A questions:</td><td>".select_from_array("answer_sort", $answer_sort_styles,  getSortStyle($user,"answer"))."</td></tr>
        </table>"
);

if ($user->link_popup==1){$forum_link_externally="checked=\"checked\"";} else {$forum_link_externally="";}
if ($user->images_as_links==1){$forum_image_as_link="checked=\"checked\"";} else {$forum_image_as_link="";}
if ($user->jump_to_unread==1){$forum_jump_to_unread="checked=\"checked\"";} else {$forum_jump_to_unread="";}
if ($user->ignore_sticky_posts==1){$forum_ignore_sticky_posts="checked=\"checked\"";} else {$forum_ignore_sticky_posts="";}

row2("Display and Behavior".
    "<br><font size=-2>How to treat links and images in the forum<br>and how to act on unread posts</font>",
    "<table><tr><td>
        <input type=\"checkbox\" name=\"forum_images_as_links\" ".$forum_image_as_link."> Show images as links<br>
        <input type=\"checkbox\" name=\"forum_link_externally\" ".$forum_link_externally."> Open links in new window/tab<br>
        <input type=\"checkbox\" name=\"forum_jump_to_unread\" ".$forum_jump_to_unread."> Jump to first new post in thread automatically<br>
        <input type=\"checkbox\" name=\"forum_ignore_sticky_posts\" ".$forum_ignore_sticky_posts.">Do not reorder sticky posts<br>
    </td></tr></table>"
);

if ($user->hide_avatars==1){$forum_hide_avatars="checked=\"checked\"";} else {$forum_hide_avatars="";}
if ($user->hide_signatures==1){$forum_hide_signatures="checked=\"checked\"";} else {$forum_hide_signatures="";}
$forum_low_rating_threshold= $user->low_rating_threshold;
$forum_high_rating_threshold= $user->high_rating_threshold;
row2("Filtering".
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

$filtered_userlist=explode("|",$user->ignorelist);
for ($i=1;$i<sizeof($filtered_userlist);$i++){
    $filtered_user = lookup_user_id($filtered_userlist[$i]);
    $forum_filtered_userlist.="<input type =\"submit\" name=\"remove".$filtered_userlist[$i]."\" value=\"Remove\"> ".$filtered_userlist[$i]." - ".user_links($filtered_user,URL_BASE)."<br>";
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



if ($user->no_signature_by_default==0){$enable_signature="checked=\"checked\"";} else {$enable_signature="";}
$signature=stripslashes($user->signature);
$maxlen=250;
row2("Signature for message boards".html_info().
    "<font size=-2><br>Max length (including newlines) is $maxlen chars.</font>",
    "<table><tr><td>
    <textarea name=\"signature\" rows=4 cols=50 id=\"signature\" onkeydown=\"textCounter(this.form.signature, this.form.remLen,$maxlen);\"
    onkeyup=\"textCounter(this.form.signature, this.form.remLen,250);\">".$signature."</textarea>
    <br><input name=\"remLen\" type=\"text\" id=\"remLen\" value=\"".($maxlen-strlen($signature))."\" size=\"3\" maxlength=\"3\" readonly> chars remaining
    <br><input type=\"checkbox\" name=\"signature_enable\" ".$enable_signature."> Attach signature by default
    </td></tr></table>");
if ($user->signature!=""){
row2("Signature preview".
    "<br><font size=-2>This is how your signature will look in the forums</font>",
    nl2br(sanitize_html(stripslashes($user->signature)))
);
}
row2("", "<input type=submit value='Update info'>");
echo "</form>\n";
end_table();
page_tail();
?>
