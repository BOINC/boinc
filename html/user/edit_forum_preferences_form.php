<?php

require_once("../inc/db.inc");
require_once("../inc/forum.inc");
require_once("../inc/util.inc");

db_init();
$user = get_logged_in_user();
$user = getForumPreferences($user);

page_head("Edit message board preferences");

start_table();
row2("Reset preferences<br><font size=-2>Use this button to reset preferences to the defaults</font>",
    "<form method=post action=\"edit_forum_preferences_action.php\"><input type=\"submit\" value=\"Reset preferences\"></form>");
echo "<form method=\"post\" action=\"edit_forum_preferences_action.php\" enctype=\"multipart/form-data\">";



if ($user->avatar_type==0){
    $zero_select="checked=true";
} elseif($user->avatar_type==1){
    $one_select="checked=true";
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

row2("Sort styles<br><font size=-2>How to sort the replies in the message board and Q&A areas</font>",
    "
        <table>
            <tr><td>Message threadlist:</td><td>".select_from_array("forum_sort", $forum_sort_styles, getSortStyle($user,"forum"))."</td></tr>
            <tr><td>Message posts:</td><td>".select_from_array("thread_sort", $thread_sort_styles, getSortStyle($user,"thread"))."</td></tr>
            <tr><td>Q&amp;A questionlist:</td><td>".select_from_array("faq_sort", $faq_sort_styles,  getSortStyle($user,"faq"))."</td></tr>
            <tr><td>Q&amp;A questions:</td><td>".select_from_array("answer_sort", $answer_sort_styles,  getSortStyle($user,"answer"))."</td></tr>
        </table>"
);

if ($user->link_popup==1){$forum_link_externally="checked=\"true\"";} else {$forum_link_externally="";}
if ($user->images_as_links==1){$forum_image_as_link="checked=\"true\"";} else {$forum_image_as_link="";}
if ($user->jump_to_unread==1){$forum_jump_to_unread="checked=\"true\"";} else {$forum_jump_to_unread="";}

row2("Display and Behavior".
    "<br><font size=-2>How to treat links and images in the forum<br>and how to act on unread posts</font>",
    "<table><tr><td>
        <input type=\"checkbox\" name=\"forum_images_as_links\" ".$forum_image_as_link."> Show images as links<br>
        <input type=\"checkbox\" name=\"forum_link_externally\" ".$forum_link_externally."> Open links in new window/tab<br>
        <input type=\"checkbox\" name=\"forum_jump_to_unread\" ".$forum_jump_to_unread."> Jump to first new post in thread automatically<br>
    </td></tr></table>"
);

if ($user->hide_avatars==1){$forum_hide_avatars="checked=\"true\"";} else {$forum_hide_avatars="";}
if ($user->hide_signatures==1){$forum_hide_signatures="checked=\"true\"";} else {$forum_hide_signatures="";}
$forum_low_rating_threshold= $user->low_rating_threshold;
$forum_high_rating_threshold= $user->high_rating_threshold;
row2("Filtering".
    "<br><font size=-2>What to display<br>If you set both your high and low thresholds to 0 or<br>empty they will reset to the default values</font>",
    "<table><tr><td>
        <input type=\"checkbox\" name=\"forum_hide_avatars\" ".$forum_hide_avatars."> Hide avatar images<br>
        <input type=\"checkbox\" name=\"forum_hide_signatures\" ".$forum_hide_signatures."> Hide signatures<br>
    </td></tr></table>
    <table width=\"380\">
	<tr><td width=\"32\"><input type=\"input\" name=\"forum_low_rating_threshold\" value=\"".$forum_low_rating_threshold."\" style=\"width: 30px;\"></td><td>Filter threshold (default: ".DEFAULT_LOW_RATING_THRESHOLD.")</td></tr>
        <tr><td><input type=\"input\" name=\"forum_high_rating_threshold\" value=\"".$forum_high_rating_threshold."\" style=\"width: 30px;\"></td><td>Emphasize threshold (default: ".DEFAULT_HIGH_RATING_THRESHOLD.")</td></tr>
	<tr><td colspan=2>
	    Anything rated lower than the filter threshold will be filtered and anything rated higher than the emphasize threshold will be emphasized.
	</td></tr>	
    </table>
    "
);


if ($user->no_signature_by_default==0){$enable_signature="checked=\"true\"";} else {$enable_signature="";}
row2("Signature for message boards".
    "<br><a href=html.php><font size=-2>May contain HTML tags</font></a><font size=-2><br>Max length (including newlines) is 250 chars.",
    "
        <textarea name=signature rows=4 cols=50>".stripslashes($user->signature)."</textarea>
        <br><input type=\"checkbox\" name=\"signature_enable\" ".$enable_signature."> Attach signature by default"
);
if ($user->signature!=""){
row2("Signature preview".
    "<br><font size=-2>This is how your signature will look in the forums</font>",
    sanitize_html(stripslashes($user->signature))
);
}
row2("", "<input type=submit value='Update info'>");
end_table();
echo "</form>\n";
page_tail();

?>
