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

require_once("../inc/util.inc");
require_once("../inc/forum.inc");
require_once("../inc/image.inc"); // Avatar scaling

if (post_str("account_key", true) != null) {
    $user = BoincUser::lookup_auth(post_str("account_key"));
    $rpc = true;
} else {
    $user = get_logged_in_user();
    $rpc = false;
}
BoincForumPrefs::lookup($user);

if (post_str("action", true)=="reset_confirm"){
    page_head(tra("Confirm reset"));
    echo tra("This action will erase any changes you have made in your community preferences. To cancel, click your browser's Back button.")."
        <p>
        <form action=edit_forum_preferences_action.php method=post>
        <input type=hidden name=action value=reset>
        <input class=\"btn btn-warning\" type=submit value=\"".tra("Reset preferences")."\">
        </form>
    ";
    page_tail();
    exit();
}

// If the user has requested a reset of preferences;
// preserve a few fields.
//
if (post_str("action", true)=="reset"){
    $posts = $user->prefs->posts;
    $last_post = $user->prefs->last_post;
    $rated_posts = $user->prefs->rated_posts;
    $banished_until = $user->prefs->banished_until;
    $special_user = $user->prefs->special_user;
    $user->prefs->delete();
    unset($user->prefs);
    BoincForumPrefs::lookup($user, true);
    $user->prefs->update("posts=$posts, last_post=$last_post, rated_posts='$rated_posts', banished_until=$banished_until, special_user='$special_user'");
    Header("Location: edit_forum_preferences_form.php");
    exit;
}

$pmn = post_int("pm_notification");
if ($pmn != $user->prefs->pm_notification) {
    $user->prefs->update("pm_notification=$pmn");
}

if (!DISABLE_FORUMS) {

$avatar_type = post_int("avatar_select");
$newfile=IMAGE_PATH.$user->id."_avatar.jpg";

// Update the user avatar
if ($avatar_type<0 or $avatar_type>3) $avatar_type=0;
if ($avatar_type==0){
    if (file_exists($newfile)){
        // Delete the file on the server if the user
        // decides not to use an avatar
        //
        unlink($newfile);
    }
    $avatar_url="";
} elseif ($avatar_type == 1) {
    $avatar_url = "//www.gravatar.com/avatar/".md5($user->email_addr)."?s=100&amp;d=identicon";
} elseif ($avatar_type==2){
    if (($rpc && (post_str("avatar_url", true) != null)) || ($_FILES['picture']['tmp_name']!="")) {
        if ($_FILES['picture']['tmp_name']!="") {
            $file = $_FILES['picture']['tmp_name'];
        } else {
            // Remote image. Download and store locally
            $file = post_str("avatar_url");
        }
        $size = getImageSize($file);
        if ($size[2]!=2 and $size[2]!=3){
            //Not the right kind of file
            error_page(tra("Error: Not the right kind of file, only PNG and JPEG are supported."));
        }
        $width = $size[0];
        $height = $size[1];
        $image2 = intelligently_scale_image($file, 100, 100);
        ImageJPEG($image2, $newfile);
    }
    if (file_exists($newfile)){
        $avatar_url=IMAGE_URL.$user->id."_avatar.jpg"; //$newfile;
    } else {
        //User didn't upload a compatible file or it went lost on the server
        $avatar_url="";
    }
}

$images_as_links = (isset($_POST["forum_images_as_links"]) && $_POST["forum_images_as_links"]!="")?1:0;
$link_popup = (isset($_POST["forum_link_popup"]) && $_POST["forum_link_popup"]!="")?1:0;
$hide_avatars = (isset($_POST["forum_hide_avatars"]) && $_POST["forum_hide_avatars"]!="")?1:0;
$hide_signatures = (isset($_POST["forum_hide_signatures"]) && $_POST["forum_hide_signatures"]!="")?1:0;
$highlight_special = (isset($_POST["forum_highlight_special"]) && $_POST["forum_highlight_special"]!="")?1:0;
$jump_to_unread = (isset($_POST["forum_jump_to_unread"]) && $_POST["forum_jump_to_unread"]!="")?1:0;
$ignore_sticky_posts = (isset($_POST["forum_ignore_sticky_posts"]) && $_POST["forum_ignore_sticky_posts"]!="")?1:0;
$no_signature_by_default = (isset($_POST["signature_by_default"]) && $_POST["signature_by_default"]!="")?0:1;
$signature = post_str("signature", true);
if (strlen($signature)>250) {
    error_page(tra("Your signature was too long, please keep it less than 250 characters."));
}
$forum_sort = post_int("forum_sort");
$thread_sort = post_int("thread_sort");
$display_wrap_postcount = post_int("forum_display_wrap_postcount");
if ($display_wrap_postcount<1) $display_wrap_postcount=1;

$signature = BoincDb::escape_string($signature);

$user->prefs->update("images_as_links=$images_as_links, link_popup=$link_popup, hide_avatars=$hide_avatars, hide_signatures=$hide_signatures, highlight_special=$highlight_special, jump_to_unread=$jump_to_unread, ignore_sticky_posts=$ignore_sticky_posts, no_signature_by_default=$no_signature_by_default, avatar='$avatar_url', signature='$signature', forum_sorting=$forum_sort, thread_sorting=$thread_sort, display_wrap_postcount=$display_wrap_postcount");

}   // DISABLE_FORUMS

$add_user_to_filter = (isset($_POST["add_user_to_filter"]) && $_POST["add_user_to_filter"]!="");
if ($add_user_to_filter){
    $user_to_add = trim($_POST["forum_filter_user"]);
    if ($user_to_add!="" and $user_to_add==strval(intval($user_to_add))){
        $other_user = BoincUser::lookup_id($user_to_add);
        if (!$other_user) {
            echo tra("No such user:")." ".$user_to_add;
        } else {
            add_ignored_user($user, $other_user);
        }
    }
}

// Or remove some from the ignore list
//
$ignored_users = get_ignored_list($user);
for ($i=0;$i<sizeof($ignored_users);$i++){
    $remove = "remove".trim($ignored_users[$i]);
    if (isset($_POST[$remove]) && $_POST[$remove]!=""){
        $other_user = BoincUser::lookup_id($ignored_users[$i]);
        if (!$other_user) {
            echo tra("No such user:")." ".$ignored_users[$j];
        } else {
            remove_ignored_user($user, $other_user);
        }
    }
}


if ($rpc == false) {
    // If we get down here everything went ok
    // redirect the user to the setup page again
    //
    Header("Location: edit_forum_preferences_form.php");
} else {
    echo "<status>\n";
    echo "    <success>1</success>\n";
    echo "</status>\n";
}

?>
