<?php

// commit the chanegs made in edit_forum_preferences_form.php to the database.


require_once("../inc/forum.inc");
require_once("../inc/image.inc"); // Avatar scaling

if (post_str("account_key", true) != null) {
    $user = lookup_user_auth(post_str("account_key"));
    $rpc = true;
} else {
    $user = get_logged_in_user();
    $rpc = false;
}
BoincForumPrefs::lookup($user);

// If the user has requested a reset of preferences;
// preserve a few fields.
//
if (post_str("action", true)=="reset"){
    $posts = $user->prefs->posts;
    $banished_until = $user->prefs->banished_until;
    $special_user = $user->prefs->special_user;
    $user->prefs->delete();
    BoincForumPrefs::lookup($user);
    $user->prefs->update("posts=$posts, banished_until=$banished_until, special_user=$special_user");
    Header("Location: edit_forum_preferences_form.php");
    exit;
}

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
            error_page("Error: Not the right kind of file, only PNG and JPEG are supported.");
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

// Update some simple prefs that are either on or off
$images_as_links = ($_POST["forum_images_as_links"]!="")?1:0;
$link_popup = ($_POST["forum_link_popup"]!="")?1:0;
$hide_avatars = ($_POST["forum_hide_avatars"]!="")?1:0;
$hide_signatures = ($_POST["forum_hide_signatures"]!="")?1:0;
$jump_to_unread = ($_POST["forum_jump_to_unread"]!="")?1:0;
$ignore_sticky_posts = ($_POST["forum_ignore_sticky_posts"]!="")?1:0;
$no_signature_by_default = ($_POST["signature_by_default"]!="")?0:1;
$pm_notification = ($_POST["pm_notification"]!="")?1:0;
$low_rating_threshold = post_int("forum_low_rating_threshold");
$high_rating_threshold = post_int("forum_high_rating_threshold");
$signature = stripslashes($_POST["signature"]);
if (strlen($signature)>250) {
    error_page(
        "Your signature was too long, please keep it less than 250 chars"
    );
}
$forum_sort = post_int("forum_sort");
$thread_sort = post_int("thread_sort");
$minimum_wrap_postcount = post_int("forum_minimum_wrap_postcount");
$display_wrap_postcount = post_int("forum_display_wrap_postcount");
if ($minimum_wrap_postcount<0) $minimum_wrap_postcount=0;
if ($display_wrap_postcount>$minimum_wrap_postcount) {
    $display_wrap_postcount=round($minimum_wrap_postcount/2);
}
if ($display_wrap_postcount<5) $display_wrap_postcount=5;

$signature = BoincDb::escape_string($signature);

$user->prefs->update("images_as_links=$images_as_links, link_popup=$link_popup, hide_avatars=$hide_avatars, hide_signatures=$hide_signatures, jump_to_unread=$jump_to_unread, ignore_sticky_posts=$ignore_sticky_posts, no_signature_by_default=$no_signature_by_default, pm_notification=$pm_notification, avatar='$avatar_url', low_rating_threshold=$low_rating_threshold, high_rating_threshold=$high_rating_threshold, signature='$signature', forum_sorting=$forum_sort, thread_sorting=$thread_sort, minimum_wrap_postcount=$minimum_wrap_postcount, display_wrap_postcount=$display_wrap_postcount");


$add_user_to_filter = ($_POST["add_user_to_filter"]!="");
if ($add_user_to_filter){
    $user_to_add = trim($_POST["forum_filter_user"]);
    if ($user_to_add!="" and $user_to_add==strval(intval($user_to_add))){
        $other_user = BoincUser::lookup_id($user_to_add);
        if (!$other_user) {
            echo "No such user: $other_user";
        } else {
            add_ignored_user($user, $other_user);
        }
    }
}

// Or remove some from the ignore list
//
$ignored_users = get_ignored_list($user);
for ($i=0;$i<sizeof($ignored_users);$i++){
    if ($_POST["remove".trim($ignored_users[$i])]!=""){
        $other_user = BoincUser::lookup_id($user_to_add);
        if (!$other_user) {
            echo "No such user: $other_user";
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
